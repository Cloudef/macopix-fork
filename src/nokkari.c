//  MaCoPiX = Mascot Construnctive Pilot for X
//                                (ActX / Gtk+ Evolution)
//
//
//      nokkari.c  
//      Nokkari-Chara (an Window Sitter for MS Win) Converter
//                   (for Nokkari-chara ver1.40 or later)
//
//                            Copyright 2002-2007  K.Chimari
//                                     http://rosegray.sakura.ne.jp/
//
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
//


#include "main.h"
#include "nokkari.h"
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <signal.h>

#ifndef USE_GTK2
#include <png.h>
#endif

#if HAVE_ICONV
#  include <iconv.h>
#endif

#define CODE_TEST

#define BUFFSIZE 4096

void NkrSave();
void NkrRead();
gint anim_loop();
void remove_anim_loop();

#ifndef USE_GTK2
void WritePNG();
#endif

#if HAVE_ICONV
void CodeHenkanSJIS();
void CodeHenkanEUC();
#endif

void create_nkr_dialog();
static void close_nkr();

extern gchar* to_locale();
extern gchar* to_utf8();

extern int WriteBMP ();

extern gchar* create_nkr_change_image_dialog();

extern void copy_file();

#if HAVE_ICONV
extern gchar *conv_iconv_strdup();
#endif

#ifdef __GTK_STOCK_H__
extern GtkWidget* gtkut_button_new_from_stock();
#endif

extern void my_signal_connect();
extern gchar* my_dirname();
extern gchar* my_basename();
extern gboolean my_main_iteration();

int total_ptn;
gchar *rdm_ptn, *clk_ptn;

gchar *nkr_msg;


extern typMascot *Mascot;

extern gchar* FullPathPixmapFile();

// �Τä��ꥭ����� ini�ե�����Υ�����
void NkrSave (typMascot *mascot)
{
  ConfigFile *cfgfile;
  gchar *filename=NULL;
  gchar tmp[64], *tmp1=NULL, *tmp2=NULL, tmp_msg[256];
  int i_pix=0, i_ptn=0, i_frm=0, i_tmp;
  gchar *p=NULL;
  FILE *fp;
  GdkPixbuf *pixbuf = NULL;

  total_ptn=0;
  rdm_ptn=NULL;
  clk_ptn=NULL;
  nkr_msg=NULL;

  filename = g_strdup(mascot->inifile);
  cfgfile = xmms_cfg_open_file(filename);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  
  // �����ȥ�
  sprintf(tmp,"%s [MaCoPiX]",mascot->name);
  xmms_cfg_write_string(cfgfile, "Chara", "Name",tmp);

  // �����ե�����
  for(i_pix=0;i_pix<mascot->nPixmap;i_pix++){
    if(mascot->sprites[i_pix].filename){
      sprintf(tmp, "File%d", i_pix+1);
      tmp2=g_strdup(my_basename(mascot->sprites[i_pix].filename));
      p=(char *)strstr(tmp2,".png");
      strcpy(p,".bmp");
      
      // BMP�ե�����ν񤭽Ф�
#ifdef USE_GTK2
      pixbuf = gdk_pixbuf_new_from_file(mascot->sprites[i_pix].filename,NULL);
#else
      pixbuf = gdk_pixbuf_new_from_file(mascot->sprites[i_pix].filename);
#endif
      if(pixbuf==NULL){
	g_print (_("Cannot Load Image %s\n"), mascot->sprites[i_pix].filename);
	exit(1);
      }
      tmp1=g_strconcat(my_dirname(mascot->inifile),G_DIR_SEPARATOR_S,tmp2,NULL);
      WriteBMP (tmp1, pixbuf);
#ifdef USE_GTK2
      g_object_unref(G_OBJECT(pixbuf));
#else
      gdk_pixbuf_unref(pixbuf);
#endif
      
      if(mascot->sprites[i_pix].filename){
	xmms_cfg_write_string(cfgfile, "Pat", tmp, tmp2);
	if(i_pix>NKR_MAX_FILE-1){
	  sprintf(tmp_msg,
		  _("Warning : Too much BMP files for Nokkari-Chara! [File%d]\n"),
		    i_pix+1);
	  if(nkr_msg)  nkr_msg=g_strconcat(nkr_msg,tmp_msg,NULL);
	  else         nkr_msg=g_strdup(tmp_msg);
	}
      }
      else{
	xmms_cfg_remove_key(cfgfile, "Pat", tmp);
      }
    }
  }

  i_tmp=i_pix;
  for(i_pix=i_tmp;i_pix<NKR_MAX_FILE;i_pix++){
      sprintf(tmp, "File%d", i_pix+1);
      xmms_cfg_remove_key(cfgfile, "Pat", tmp);
  }


  // �ݥ������
  sprintf(tmp,"%d",mascot->xoff);
  xmms_cfg_write_string(cfgfile, "Pos", "X",tmp);
  sprintf(tmp,"%d",mascot->height-mascot->yoff);
  xmms_cfg_write_string(cfgfile, "Pos", "Y",tmp);
  

  // ���˥᡼�����
  i_ptn=0;
  while(anim_loop(mascot,i_ptn,cfgfile)==-1){
    i_ptn++;
  }
  
  for(i_ptn=total_ptn;i_ptn<NKR_MAX_PAT;i_ptn++){
    remove_anim_loop(i_ptn,cfgfile);
  }


  // ����å����˥�
  if(clk_ptn) xmms_cfg_write_string(cfgfile, "Click", "AnimeNo",clk_ptn);

  // �����ॢ�˥�
  if(rdm_ptn) xmms_cfg_write_string(cfgfile, "Random", "AnimeNo",rdm_ptn);

  // �������

  xmms_cfg_write_file(cfgfile, mascot->inifile);

  xmms_cfg_free(cfgfile);

  
  if((fp=fopen(mascot->inifile,"a"))==NULL){
    g_message(_("Cannot Save Ini File!"));
    exit(1);
  }
   
  fprintf(fp,"\n[EOF]\n");
    
  fclose(fp);

#if HAVE_ICONV
  CodeHenkanSJIS(mascot);
#endif

  create_nkr_dialog(mascot);
}



// ���˥᡼�����loop�κƸ� 1ptn�֤�
gint anim_loop(typMascot *mascot, int i_ptn, ConfigFile *cfgfile)
{
  int i_frm=0, i_allfrm=0, i_wgt;
  int seqend;
  int total_frm=0;
  int frm_pix[BUF_NKR_MAX_PAT_Y],frm_num[BUF_NKR_MAX_PAT_Y];
  int max_frm;
  gchar tmp[64], tmp2[64], *tmp1=NULL;
  int wp_no,wp_max,i_wp;
  gchar *wp[64];
  gchar w_tmp[1024],wp_tmp[1024],tmp_msg[256];
  int i_p;

  if(mascot->frame_pix[i_ptn][0]==-1) return(i_ptn);

  // ���˥�
  while(i_frm<mascot->frame_num[i_ptn]){

    seqend=RANDOM(mascot->frame_max[i_ptn][i_frm]
		  -mascot->frame_min[i_ptn][i_frm]+1)
      +mascot->frame_min[i_ptn][i_frm];
    total_frm+=seqend;
    frm_num[i_allfrm]=total_frm;
    frm_pix[i_allfrm]=mascot->frame_pix[i_ptn][i_frm];
    i_allfrm++;

    // �֥�å��롼��
    if((mascot->frame_loop[i_ptn][i_frm].next>=0)){
      mascot->frame_loop[i_ptn][i_frm].seq++;
      if(mascot->frame_loop[i_ptn][i_frm].seq==1){
	// �֥�å��롼�׳���
	mascot->frame_loop[i_ptn][i_frm].seqend=
	  RANDOM(mascot->frame_loop[i_ptn][i_frm].max
		 -mascot->frame_loop[i_ptn][i_frm].min+1)
	  +mascot->frame_loop[i_ptn][i_frm].min;
	       
	if(mascot->frame_loop[i_ptn][i_frm].seqend==1){
	  mascot->frame_loop[i_ptn][i_frm].seq=0;
	  i_frm++;
	}
	else{
	  i_frm=
	    mascot->frame_loop[i_ptn][i_frm].next;
	}
      }
      else if(mascot->frame_loop[i_ptn][i_frm].seqend-
	      mascot->frame_loop[i_ptn][i_frm].seq<=0){
	// �֥�å��롼�׽�λ
	mascot->frame_loop[i_ptn][i_frm].seq=0;
	i_frm++;
      }
      else{
	// �֥�å��롼�׷�³
	i_frm=
	  mascot->frame_loop[i_ptn][i_frm].next;
      }
    }
    else{

      i_frm++;
    }
  }

  max_frm=i_allfrm;

  if(mascot->random_weight[i_ptn]>0){
    sprintf(tmp,"Pat%d",total_ptn+1);
    xmms_cfg_write_int(cfgfile, "Anime", tmp, max_frm);

    // Buffer �̤��к�
    if(max_frm>BUF_NKR_MAX_PAT_Y-1){
      max_frm=BUF_NKR_MAX_PAT_Y-1;
    }

    if(max_frm>=NKR_MAX_PAT_Y){
      sprintf(tmp_msg,
	      _("Warning : Too much Frames!! [%s=%d]\n"),tmp,max_frm);
      if(nkr_msg)  nkr_msg=g_strconcat(nkr_msg,tmp_msg,NULL);
      else         nkr_msg=g_strdup(tmp_msg);
    }

    for(i_frm=0;i_frm<max_frm;i_frm++){
      sprintf(tmp,"Pat%d_%d",total_ptn+1,i_frm+1);
      sprintf(tmp2,"%d %d",frm_pix[i_frm]+1,frm_num[i_frm]+1);
      xmms_cfg_write_string(cfgfile, "Anime", tmp, tmp2);
    }

    for(i_frm=max_frm;i_frm<NKR_MAX_PAT_Y;i_frm++){
      sprintf(tmp,"Pat%d_%d",total_ptn+1,i_frm+1);
      xmms_cfg_remove_key(cfgfile, "Anime", tmp);
    }
    
    sprintf(tmp,"%d ",total_ptn+1);
    for(i_wgt=0;i_wgt<mascot->random_weight[i_ptn];i_wgt++){
      if(rdm_ptn) rdm_ptn=g_strconcat(rdm_ptn,tmp,NULL);
      else rdm_ptn=g_strdup(tmp);
    }

    total_ptn++;
  }
  

  if(mascot->click_weight[i_ptn]>0){
    sprintf(tmp,"Pat%d",total_ptn+1);
    xmms_cfg_write_int(cfgfile, "Anime", tmp, max_frm);

    // Buffer �̤��к�
    if(max_frm>BUF_NKR_MAX_PAT_Y-1){
      max_frm=BUF_NKR_MAX_PAT_Y-1;
    }

    if(max_frm>=NKR_MAX_PAT_Y){
      sprintf(tmp_msg,
	      _("Warning : Too much Frames!! [%s=%d]\n"),tmp, max_frm);
      if(nkr_msg)  nkr_msg=g_strconcat(nkr_msg,tmp_msg,NULL);
      else         nkr_msg=g_strdup(tmp_msg);
    }

    for(i_frm=0;i_frm<max_frm;i_frm++){
      
      sprintf(tmp,"Pat%d_%d",total_ptn+1,i_frm+1);
      sprintf(tmp2,"%d %d",frm_pix[i_frm]+1,frm_num[i_frm]+1);
      xmms_cfg_write_string(cfgfile, "Anime", tmp, tmp2);
    }

    for(i_frm=max_frm;i_frm<NKR_MAX_PAT_Y;i_frm++){
      sprintf(tmp,"Pat%d_%d",total_ptn+1,i_frm+1);
      xmms_cfg_remove_key(cfgfile, "Anime", tmp);
    }


    // �դ�����
    
    if(mascot->click_word[i_ptn]) {
      strcpy(w_tmp,mascot->click_word[i_ptn]);
      wp_no=0;
      wp[wp_no]=(char *)strtok(w_tmp,"%");
      for(;;){
	wp_no++;
	if((wp[wp_no]=(char *)strtok(NULL,"%"))==NULL){
	  wp_max=wp_no;
	  break;
	}
	if((wp[wp_no][0]=='p')||(wp[wp_no][0]=='n')||
	   (wp[wp_no][0]=='a')){
	  wp[wp_no]++;
	}
	else if((wp[wp_no][0]=='m')||(wp[wp_no][0]=='c')){
	  strcpy(wp_tmp,&wp[wp_no][1]);
	  strcat(wp[wp_no-1],wp_tmp);
	  wp_no--;
	}
	else if(isdigit(wp[wp_no][0])){
	  i_p=0;
	  for(;;){
	    i_p++;
	    if(isdigit(wp[wp_no][i_p])){
	    }
	    else if((wp[wp_no][i_p]=='p')||(wp[wp_no][i_p]=='a')||
		    (wp[wp_no][i_p]=='n')){
	      wp[wp_no]+=i_p+1;
	      break;
	    }
	    else if((wp[wp_no][i_p]=='c')||(wp[wp_no][i_p]=='m')){
	      strcpy(wp_tmp,&wp[wp_no][i_p+1]);
	      strcat(wp[wp_no-1],wp_tmp);
	      wp_no--;
	      break;
	    }
	  }
	}
	else{
	  strcpy(wp_tmp,wp[wp_no]);
	  strcat(wp[wp_no-1],"%");
	  strcat(wp[wp_no-1],wp_tmp);
	  wp_no--;
	}
      }
      if(wp_max>3){
	sprintf(tmp_msg,
		_("Warning : Too much Line numbers for the Balloon message! [Pat%d]\n"),
		total_ptn+1);
	if(nkr_msg)  nkr_msg=g_strconcat(nkr_msg,tmp_msg,NULL);
	else         nkr_msg=g_strdup(tmp_msg);
      }

      tmp1=NULL;
      for(i_wp=0;i_wp<wp_max;i_wp++){
	if(i_wp!=0){
	  if(tmp1) tmp1=g_strconcat(tmp1," ",NULL);
	}
	
	if(tmp1) tmp1=g_strconcat(tmp1,wp[i_wp],NULL);
	else tmp1=g_strdup(wp[i_wp]);
      }

      if(tmp1){
	sprintf(tmp,"Pat%d",total_ptn+1);
	xmms_cfg_write_string(cfgfile, "Serifu", tmp, tmp1);
      }
      else{
	sprintf(tmp,"Pat%d",total_ptn+1);
	xmms_cfg_remove_key(cfgfile, "Serifu", tmp);
      }
    }
    else{
      sprintf(tmp,"Pat%d",total_ptn+1);
      xmms_cfg_remove_key(cfgfile, "Serifu", tmp);
    }

    sprintf(tmp,"%d ",total_ptn+1);
    for(i_wgt=0;i_wgt<mascot->random_weight[i_ptn];i_wgt++){
      if(clk_ptn) clk_ptn=g_strconcat(clk_ptn,tmp,NULL);
      else clk_ptn=g_strdup(tmp);
    }

    total_ptn++;
  } 

  return(-1);
}

// ���˥᡼�����loop�κ�� 1ptn�֤� (�Ťͽ��к�)
void remove_anim_loop(int i_ptn, ConfigFile *cfgfile)
{
  int i_frm=0;
  gchar tmp[64];

  sprintf(tmp,"Pat%d",i_ptn+1);
  xmms_cfg_write_int(cfgfile, "Anime", tmp, 0);
  xmms_cfg_remove_key(cfgfile, "Serifu", tmp);

  for(i_frm=0;i_frm<BUF_NKR_MAX_PAT_Y;i_frm++){
    sprintf(tmp,"Pat%d_%d",i_ptn+1,i_frm+1);
    xmms_cfg_remove_key(cfgfile, "Anime", tmp);
  }
  
}



// �Τä��ꥭ���ե�������ɤ߹���
void NkrRead(typMascot *mascot)
{
  ConfigFile *cfgfile;
  gchar *inifilename=NULL, *filename2=NULL, *filename0=NULL;
  gchar tmp[64], tmp0[64], *tmp1=NULL,*tmp2=NULL,tmp_msg[256];
  int i_pix=0, i_ptn=0, i_frm=0;
  gint i_tmp;
  gchar *p, *wp;
  gint cur_frm,old_frm;
  GdkPixbuf *pixbuf = NULL, *pixbuf2 = NULL;
  gchar *tmp_open;
  
  nkr_msg=NULL;
  mascot->random_total=0;
  mascot->click_total=0;
  // mascot->flag_consow=FALSE;
  
  // Ini�ե������ľ��dir�����
  inifilename = g_strdup(mascot->inifile);
  if(access(inifilename,F_OK)!=0){
    g_message(_("Cannot Find Mascot File %s\n"),my_basename(mascot->inifile));
    exit(1);
  }

#if HAVE_ICONV
  CodeHenkanEUC(mascot);
#endif

  /*
  mascot->file=g_strdup(filename);
  */
  cfgfile = xmms_cfg_open_file(inifilename);
  
  
  
  if (cfgfile) {
    // General
    if(!xmms_cfg_read_string(cfgfile, "Chara", "Name",&mascot->name))
      mascot->name=NULL;
    mascot->copyright=NULL;
    mascot->code=g_strdup(MACOPIX_CODE);


    // Focus Movement etc.
    
    mascot->move=MOVE_FOCUS;
    if(!xmms_cfg_read_int(cfgfile, "Pos", "X",&mascot->xoff))
      mascot->xoff=0;
    if(!xmms_cfg_read_int(cfgfile, "Pos", "Y",&mascot->yoff))
      mascot->yoff=0;
    mascot->x=0;
    mascot->y=0;

    // Clock
    mascot->clk_x=INIT_CLK_POS;
    mascot->clk_y=INIT_CLK_POS;
    mascot->clktext_x=INIT_CLK_TEXT;
    mascot->clktext_x=INIT_CLK_TEXT;
    mascot->wclkbd=INIT_CLK_BORDER;
    mascot->clkmode=CLOCK_NO;
    mascot->clktype=CLOCK_TYPE_24S;
    mascot->flag_clksd=TRUE;
    mascot->fontname_clk=NULL;
    //      mascot->fontname_clk=g_strconcat(FONT_CLK,NULL);


    // Balloon
    mascot->baltext_x=INIT_BAL_TEXT;
    mascot->baltext_y=INIT_BAL_TEXT;
    mascot->wbalbd=INIT_BAL_BORDER;
    mascot->bal_defpos=BAL_POS_LEFT;
    mascot->fontname_bal=NULL;
    //      mascot->fontname_bal=g_strconcat(FONT_BAL,NULL);

    // Color for Clock
    mascot->colclk->red=COLOR_CLK_R;
    mascot->colclk->green=COLOR_CLK_G;
    mascot->colclk->blue=COLOR_CLK_B;
    mascot->colclksd->red=COLOR_CLKSD_R;
    mascot->colclksd->green=COLOR_CLKSD_G;
    mascot->colclksd->blue=COLOR_CLKSD_B;
    mascot->colclkbg->red=COLOR_CLKBG_R;
    mascot->colclkbg->green=COLOR_CLKBG_G;
    mascot->colclkbg->blue=COLOR_CLKBG_B;
    mascot->colclkbd->red=COLOR_CLKBD_R;
    mascot->colclkbd->green=COLOR_CLKBD_G;
    mascot->colclkbd->blue=COLOR_CLKBD_B;



    // Color for Balloon
    mascot->colbal->red=COLOR_BAL_R; 
    mascot->colbal->green=COLOR_BAL_G;
    mascot->colbal->blue=COLOR_BAL_B;
    mascot->colbalbg->red=COLOR_BALBG_R;
    mascot->colbalbg->green=COLOR_BALBG_G;
    mascot->colbalbg->blue=COLOR_BALBG_B;
    mascot->colbalbd->red=COLOR_BALBD_R;
    mascot->colbalbd->green=COLOR_BALBD_G;
    mascot->colbalbd->blue=COLOR_BALBD_B;

#ifdef USE_BIFF
    // Biff�ѥǡ���
    mascot->mail.pix_file=NULL;
    mascot->mail.pix_pos=MAIL_PIX_RIGHT;
    mascot->mail.pix_x=0;
    mascot->mail.pix_y=0;
    mascot->mail.sound=NULL;
#endif


    // Pixmap�ǡ���
    for(i_pix=0;i_pix<MAX_PIXMAP;i_pix++){
      mascot->sprites[i_pix].pixmap=NULL;
      mascot->sprites[i_pix].mask=NULL;
      sprintf(tmp, "File%d", i_pix+1);
      if(!xmms_cfg_read_string(cfgfile, "Pat", tmp, &tmp1)){
      	mascot->sprites[i_pix].filename=NULL;
	break;
      }
      else{//�ѥ����󵭽Ҥ����ä���
	filename0=g_strconcat(my_dirname(mascot->inifile),G_DIR_SEPARATOR_S,tmp1,NULL);
	if(strlen(my_basename(tmp1))!=0){
	  if(access(filename0,F_OK)!=0){
	    while(!(filename0=
		    g_strdup(create_nkr_change_image_dialog(tmp1,i_pix)))){
	    }
	  }
	  tmp_open=to_utf8(filename0);
#ifdef USE_GTK2
	  pixbuf = gdk_pixbuf_new_from_file(tmp_open,NULL);
#else
	  pixbuf = gdk_pixbuf_new_from_file(tmp_open);
#endif
	  // BMP���Ф�Alpha��ȴ����ʬ��
	  pixbuf2=gdk_pixbuf_add_alpha(pixbuf,TRUE,0x00,0xFF,0x00);
	  tmp2=g_strdup(my_basename(tmp1));
	  p=(char *)strstr(tmp2,".bmp");
	  strcpy(p,".png");
	  filename0=g_strconcat(my_dirname(mascot->inifile),G_DIR_SEPARATOR_S,tmp2,NULL);
#ifdef USE_GTK2	  
	  gdk_pixbuf_save(pixbuf2,filename0,"png",NULL,NULL);
#else
	  WritePNG(pixbuf2,filename0);
#endif
#ifdef USE_GTK2
	  g_object_unref(G_OBJECT(pixbuf));
	  g_object_unref(G_OBJECT(pixbuf2));
#else
	  gdk_pixbuf_unref(pixbuf);
	  gdk_pixbuf_unref(pixbuf2);
#endif
	  g_free(tmp_open);

	  mascot->sprites[i_pix].filename=
	    (gchar *)FullPathPixmapFile(mascot, filename0);
	}
	else{
	  // "File%d=" �Ȥ���������Ƥ���
	  mascot->sprites[i_pix].filename=NULL;
	}
      }

    }

    // Animation�ǡ���

    // ����ѥ�����
    mascot->click_word[0]=NULL;
    mascot->click_sound[0]=NULL;

    mascot->frame_pix[0][0]=0;
    mascot->frame_min[0][0]=1;
    mascot->frame_max[0][0]=100;
    mascot->frame_loop[0][0].next=-1;
    mascot->frame_num[0]=1;

    // �ѥ�����1�ʹ�
    // �ƥѥ�����Υե졼��0��pix�ե�����̾���ʤ��Ȥ�������
    for(i_ptn=1;i_ptn<MAX_ANIME_PATTERN;i_ptn++){
      sprintf(tmp0, "Pat%d", i_ptn);
      if(xmms_cfg_read_int(cfgfile, "Anime", tmp0, &mascot->frame_num[i_ptn])){
	if(mascot->frame_num[i_ptn]>MAX_ANIME_FRAME){
	  sprintf(tmp_msg,
		  _("Warning : Too much Frames for MaCoPiX! [Pattern%d]\n"),
		  i_ptn);
	    if(nkr_msg)  nkr_msg=g_strconcat(nkr_msg,tmp_msg,NULL);
	    else         nkr_msg=g_strdup(tmp_msg);
	}
	else if(mascot->frame_num[i_ptn]<=0){
	  break;
	}

	old_frm=1;
	
	for(i_frm=0;i_frm<mascot->frame_num[i_ptn];i_frm++){
	  sprintf(tmp0, "Pat%d_%d", i_ptn,i_frm+1);
	  if(xmms_cfg_read_string(cfgfile, "Anime", tmp0, &tmp1)){
	    sscanf(tmp1,"%d %d",
		   &mascot->frame_pix[i_ptn][i_frm],
		   &cur_frm);
	    mascot->frame_pix[i_ptn][i_frm]--;
	    mascot->frame_min[i_ptn][i_frm]=cur_frm-old_frm;
	    mascot->frame_max[i_ptn][i_frm]=cur_frm-old_frm;
	    mascot->frame_loop[i_ptn][i_frm].next=-1;
	    old_frm=cur_frm;
	  }
	  else{
	    sprintf(tmp_msg,
		    _("Error : Cannot find Frame setting! [Pat%d_%d]\n"),
		    i_ptn,i_frm+1);
	    if(nkr_msg)  nkr_msg=g_strconcat(nkr_msg,tmp_msg,NULL);
	    else         nkr_msg=g_strdup(tmp_msg);
	  }
	}

      }
      
      // �����
      sprintf(tmp0, "Pat%d", i_ptn);
      if(xmms_cfg_read_string(cfgfile, "Serifu", tmp0, &tmp1)){
	if(strlen(tmp1)!=0){
	  tmp[strlen(tmp1)-1]='\0';
      
	  wp=(gchar *)strtok(tmp1," ");
	  mascot->click_word[i_ptn]=g_strdup(wp);
	  for(;;){
	    if((wp=(gchar *)strtok(NULL," "))==NULL){
	      break;
	    }
	    else{
	      mascot->click_word[i_ptn]=
		g_strconcat(mascot->click_word[i_ptn],"%n",wp,NULL);
	    }
	  }
	}
	else{
	  mascot->click_word[i_ptn]=NULL;
	}
      }
      else{
	mascot->click_word[i_ptn]=NULL;
      }
      // �ѥ�����λ����
      //mascot->frame_pix[i_ptn][0]=-1;
    }


    // �����ॢ�˥�
    for(i_ptn=0;i_ptn<MAX_ANIME_PATTERN;i_ptn++){
      mascot->random_weight[i_ptn]=0;
      mascot->click_weight[i_ptn]=0;
    }

    if(xmms_cfg_read_string(cfgfile, "Random", "AnimeNo", &tmp1)){
      tmp[strlen(tmp1)-1]='\0';
      
      wp=(gchar *)strtok(tmp1," ");
      mascot->random_weight[atoi(wp)]++;
      mascot->random_total++;
      for(;;){
	if((wp=(gchar *)strtok(NULL," "))==NULL){
	  break;
	}
	else{
	  mascot->random_weight[atoi(wp)]++;
	  mascot->random_total++;
	}
      }
    }

    // Click���˥�
    for(i_ptn=0;i_ptn<MAX_ANIME_PATTERN;i_ptn++){
      mascot->click_weight[i_ptn]=0;
      mascot->click_weight[i_ptn]=0;
    }

    if(xmms_cfg_read_string(cfgfile, "Click", "AnimeNo", &tmp1)){
      tmp[strlen(tmp1)-1]='\0';
      
      wp=(gchar *)strtok(tmp1," ");
      mascot->click_weight[atoi(wp)]++;
      mascot->click_total++;
      for(;;){
	if((wp=(gchar *)strtok(NULL," "))==NULL){
	  break;
	}
	else{
	  mascot->click_weight[atoi(wp)]++;
	  mascot->click_total++;
	}
      }
    }



    xmms_cfg_free(cfgfile);
  }

  g_free(inifilename);
  g_free(filename0);
  g_free(filename2);
  g_free(tmp1);
  g_free(tmp2);
}

#if HAVE_ICONV

void CodeHenkanSJIS(typMascot *mascot){
  gchar *tmpname;
  FILE *src_fp, *dest_fp;
  gchar *buf, *buf2, *buf3=NULL;

  buf=g_malloc0(sizeof(gchar)*BUFFSIZE);

#ifdef USE_WIN32
  tmpname = g_strdup_printf("%s%smacopix-tmpcode.XXXXXX",
			    get_tmp_dir(), G_DIR_SEPARATOR_S);
#else
  tmpname = g_strdup_printf("%s%smacopix-tmpcode.%d.XXXXXX",
			    get_tmp_dir(), G_DIR_SEPARATOR_S,
			    getuid());
#endif

  if ((src_fp = fopen(mascot->inifile, "rb")) == NULL) {
    g_print(_("Cannot open copy source file %s"),mascot->inifile);
    exit(1);
  }

  if ((dest_fp = fopen(tmpname, "wb")) == NULL) {
    g_print(_("Cannot open copy destination file %s"),tmpname);
    exit(1);
  }


  while( fgets( buf,BUFFSIZE-2,src_fp ) != NULL ){

#ifdef USE_GTK2
    buf2=conv_iconv_strdup(buf, "UTF-8", "SJIS");
#else
    buf2=conv_iconv_strdup(buf, "EUCJP", "SJIS");
#endif
    if(buf2[strlen(buf2)-1]=='\n'){
      buf2[strlen(buf2)-1]='\r';
      buf3=g_strconcat(buf2,"\n",NULL);
    }
    fwrite(buf3, strlen(buf3), sizeof(gchar), dest_fp);
    g_free(buf2);
    if(buf3) g_free(buf3);
  }

  fclose(dest_fp);
  fclose(src_fp);

#ifndef USE_WIN32
  chmod(tmpname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#endif

  g_free(buf);

  copy_file(tmpname,mascot->inifile);
  unlink(tmpname);
}

void CodeHenkanEUC(typMascot *mascot){
  gchar *tmpname;
  FILE *src_fp, *dest_fp;
  gchar *buf, *buf2;

  buf=g_malloc0(sizeof(gchar)*BUFFSIZE);

#ifdef USE_WIN32
  tmpname = g_strdup_printf("%s%smacopix-tmpcode.XXXXXX",
			    get_tmp_dir(), G_DIR_SEPARATOR_S);
#else
  tmpname = g_strdup_printf("%s%smacopix-tmpcode.%d.XXXXXX",
			    get_tmp_dir(), G_DIR_SEPARATOR_S,
			    getuid());
#endif

  if ((src_fp = fopen(mascot->inifile, "rb")) == NULL) {
    g_print(_("Cannot open copy source file %s"),mascot->inifile);
    exit(1);
  }

  if ((dest_fp = fopen(tmpname, "wb")) == NULL) {
    g_print(_("Cannot open copy destination file %s"),tmpname);
    exit(1);
  }


  while( fgets( buf,BUFFSIZE-2,src_fp ) != NULL ){

    buf2=conv_iconv_strdup(buf, "SJIS", "EUCJP");
    if((buf2[strlen(buf2)-2]=='\r')&&(buf2[strlen(buf2)-1]=='\n')){
      buf2[strlen(buf2)-2]='\n';
      buf2[strlen(buf2)-1]='\0';
    }
    fwrite(buf2, strlen(buf2), sizeof(gchar), dest_fp);
    g_free(buf2);
  }

  fclose(dest_fp);
  fclose(src_fp);

#ifndef USE_WIN32
  chmod(tmpname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#endif

  g_free(buf);

  copy_file(tmpname,mascot->inifile);
  unlink(tmpname);
}
#endif /* HAVE_ICONV */

#ifndef USE_GTK2
// From "Linux Moe Moe project"  
void  WritePNG(GdkPixbuf *pic, gchar *fname)
{
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;

  fp = fopen(fname, "wb");
  if (fp == NULL) {
    return;
  }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				    NULL, NULL, NULL);

  if (png_ptr == NULL) {
    fclose(fp);
    return;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    return;
  }

  if (setjmp(png_ptr->jmpbuf)) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    return;
  }

  /* ----- libpng �˥��ȥ꡼����Ϥ� */
  png_init_io(png_ptr, fp);

  /* ----- png �����פ����� */
  if (gdk_pixbuf_get_has_alpha(pic) == TRUE) {
    png_set_IHDR(png_ptr, info_ptr,	
		 gdk_pixbuf_get_width(pic),
		 gdk_pixbuf_get_height(pic),
		 8,
		 PNG_COLOR_TYPE_RGB_ALPHA,
		 PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE,
		 PNG_FILTER_TYPE_BASE);
  }
  else {
    png_set_IHDR(png_ptr, info_ptr,	
		 gdk_pixbuf_get_width(pic),
		 gdk_pixbuf_get_height(pic),
		 8,
		 PNG_COLOR_TYPE_RGB,
		 PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE,
		 PNG_FILTER_TYPE_BASE);
  }

  /* ----- ����ե��᡼�����إå�����Ф� */
  png_write_info(png_ptr, info_ptr);

  /* ----- �ԥ������Ф� */
  { 
    png_uint_32  k, height;
    png_bytep row_pointers[gdk_pixbuf_get_height(pic)];
    height = gdk_pixbuf_get_height(pic);
    for(k=0; k<height; k++) {
      row_pointers[k] = gdk_pixbuf_get_pixels(pic) +
	(gdk_pixbuf_get_rowstride(pic) * k);
    }
    png_write_image(png_ptr, row_pointers);
  }

  /* ----- �񤭽Ф��ν�λ������� */
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

  fclose(fp);

  return;
}
#endif

// �Τä����Ѵ���������
void create_nkr_dialog(typMascot *mascot)
{
  GtkWidget *nkr_main;
  GtkWidget *nkr_tbl;
  GtkWidget *nkr_text;
  GtkWidget *button;
  GtkWidget *nkr_scroll;
#ifdef USE_GTK2
  GtkTextBuffer *text_buffer;
  GtkTextIter start_iter;
#endif

  // Win���ۤϽŤ��Τ����Expose���٥�����򤹤٤ƽ������Ƥ���
  while (my_main_iteration(FALSE));

  mascot->flag_menu=TRUE;

  
#ifdef USE_GTK2
  nkr_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  text_buffer = gtk_text_buffer_new(NULL);
#else
  nkr_main = gtk_window_new(GTK_WINDOW_DIALOG);
#endif

  gtk_window_set_title(GTK_WINDOW(nkr_main), 
		       _("MaCoPiX <--> Nokkari-Chara Convert Log"));
  gtk_widget_realize(nkr_main);
  my_signal_connect(nkr_main,"destroy",close_nkr, GTK_WIDGET(nkr_main));
  gtk_container_set_border_width (GTK_CONTAINER (nkr_main), 5);
  
  // 6x3�Υơ��֥�
  nkr_tbl = gtk_table_new (6, 3, FALSE);
  gtk_container_add (GTK_CONTAINER (nkr_main), nkr_tbl);

#ifdef USE_GTK2
  nkr_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(nkr_scroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  nkr_text = gtk_text_view_new_with_buffer (text_buffer);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (nkr_text), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (nkr_text), FALSE);

  gtk_container_add(GTK_CONTAINER(nkr_scroll), nkr_text);
  gtk_widget_set_usize (nkr_scroll, NKR_WIN_WIDTH, NKR_WIN_HEIGHT);

  gtk_table_attach (GTK_TABLE (nkr_tbl), nkr_scroll, 0, 5, 0, 1,
		    GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
#else
   nkr_text = gtk_text_new (NULL, NULL);
   gtk_widget_set_usize (nkr_text, NKR_WIN_WIDTH, NKR_WIN_HEIGHT);
   gtk_text_set_editable (GTK_TEXT (nkr_text), FALSE);
   gtk_table_attach_defaults (GTK_TABLE(nkr_tbl), nkr_text, 0, 5, 0, 1);

   nkr_scroll = gtk_vscrollbar_new (GTK_TEXT (nkr_text)->vadj);
   gtk_table_attach (GTK_TABLE (nkr_tbl), nkr_scroll, 5, 6, 0, 1,
		     GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
#endif

  if(nkr_msg){
    nkr_msg=g_strconcat(nkr_msg,
			_("\n\n  The above warnings/errors are found in conversion...\n\n  Please modify and use this mascot on Nokkari-Chara ver1.40 or later.\n"),
			NULL);
  }
  else{
    nkr_msg=g_strdup(_("\n Congratulations!!\n===================\n\n  You've scceeded to convert with no errors.\n\n  Please use this mascot on Nokkari-Chara ver1.40 or later.\n"));
  }

#ifdef USE_GTK2
  gtk_text_buffer_get_start_iter(text_buffer, &start_iter);
  gtk_text_buffer_insert (text_buffer, &start_iter, nkr_msg, -1);
#else
   gtk_text_insert (GTK_TEXT (nkr_text), NULL, NULL, NULL,
                  nkr_msg, -1);
#endif
 

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(_("OK"),GTK_STOCK_OK);
#else
  button=gtk_button_new_with_label(_("OK"));
#endif
  gtk_table_attach(GTK_TABLE(nkr_tbl), button, 4, 5, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect(button,"clicked",close_nkr, GTK_WIDGET(nkr_main));
  
  gtk_widget_show_all(nkr_main);
  
  gdk_flush();
}


static void close_nkr(GtkWidget *w, GtkWidget *dialog)
{
  //gdk_pointer_ungrab(GDK_CURRENT_TIME);

  while (my_main_iteration(FALSE));
  gtk_widget_destroy(GTK_WIDGET(dialog));
  while (my_main_iteration(FALSE));
 
  Mascot->flag_menu=FALSE;
  gdk_flush();
}
