#ifdef ________________________________________________________KLEINE_ERKLAERUNG
    // Verwendung für gtk Funktion gtk_fu bei gtkname GtkWidgetClass
    // Anfangs
    TS_DEF_constcharP_gtknameP(GtkWidgetClass)
    // Danach
    table_add(table_constcharP_GtkWidgetClassP, "gtk_fu",gtk_fu);
    // In der Schleife
    TS_CALL_constcharP_gtknameP(GtkWidgetClass,GTK_WIDGET_GET_CLASS,004)

    TS_DEF_constcharP_gtknameP(GtkWidgetClass)
    //evaluiert zu:
    FUS* table_constcharP_GtkWidgetClassP=table_new();
    typedef const char*(*sig_constcharP_GtkWidgetClassP)(GtkWidgetClass*);

    TS_CALL_constcharP_gtknameP(GtkWidgetClass,GTK_WIDGET_GET_CLASS,004)
    //evaluiert zu:
    if(NULL != (vu=table_get(table_constcharP_GtkWidgetClassP,command))) {
        if(DEBUG) fprintf(stderr,"CALLBACK004 %s, widget_id: %s value:",command,widget_id);
        const char* value = ((sig_constcharP_GtkWidgetClassP)vu)(GTK_WIDGET_GET_CLASS(widget));
        if(DEBUG) fprintf(stderr," %s\n",value);
        fprintf(pargs->fpout, "%s\n", value);
        fflush(pargs->fpout);
    } else
#endif // ________________________________________________________

#define TS_DEF_constcharP_gtknameP(gtkname) \
    FUS* table_constcharP_##gtkname##P=table_new(); \
    typedef const char*(*sig_constcharP_##gtkname##P)(gtkname*);
#define TS_CALL_constcharP_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_constcharP_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s value:",nr,command,widget_id); \
          const char* value = ((sig_constcharP_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %s\n",value); \
          fprintf(pargs->fpout, "%s\n", value); \
          fflush(pargs->fpout); \
      } else 

#define TS_DEF_float_gtknameP(gtkname) \
    FUS* table_float_##gtkname##P=table_new(); \
    typedef float(*sig_float_##gtkname##P)(gtkname*);
#define TS_CALL_float_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_float_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s value:",nr,command,widget_id); \
          float f_value = ((sig_float_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %d\n",f_value); \
          fprintf(pargs->fpout, "%d\n", f_value); \
          fflush(pargs->fpout); \
      } else 

#define TS_DEF_gboolean_gtknameP(gtkname) \
    FUS* table_gboolean_##gtkname##P=table_new(); \
    typedef gboolean(*sig_gboolean_##gtkname##P)(gtkname*);
#define TS_CALL_gboolean_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_gboolean_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s value:",nr,command,widget_id); \
          int i_value = ((sig_gboolean_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %i\n",i_value); \
          fprintf(pargs->fpout, "%i\n", i_value); \
          fflush(pargs->fpout); \
      } else 

#define TS_DEF_guint_gtknameP(gtkname) \
    FUS* table_guint_##gtkname##P=table_new(); \
    typedef guint(*sig_guint_##gtkname##P)(gtkname*);
#define TS_CALL_guint_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_guint_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s value:",nr,command,widget_id); \
          guint i_value = ((sig_guint_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %i\n",i_value); \
          fprintf(pargs->fpout, "%i\n", i_value); \
          fflush(pargs->fpout); \
      } else 

#define TS_DEF_int_gtknameP(gtkname) \
    FUS* table_int_##gtkname##P=table_new(); \
    typedef int(*sig_int_##gtkname##P)(gtkname*);
#define TS_CALL_int_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_int_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s value:",nr,command,widget_id); \
          int i_value = ((sig_int_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %i\n",i_value); \
          fprintf(pargs->fpout, "%i\n", i_value); \
          fflush(pargs->fpout); \
      } else 

#define TS_DEF_GtkJustification_gtknameP(gtkname) \
    FUS* table_GtkJustification_##gtkname##P=table_new(); \
    typedef GtkJustification(*sig_GtkJustification_##gtkname##P)(gtkname*);
#define TS_CALL_GtkJustification_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_GtkJustification_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s value:",nr,command,widget_id); \
          GtkJustification i_value = ((sig_GtkJustification_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %i\n",i_value); \
          fprintf(pargs->fpout, "%i\n", i_value); \
          fflush(pargs->fpout); \
      } else 

#define TS_DEF_GtkNaturalWrapMode_gtknameP(gtkname) \
    FUS* table_GtkNaturalWrapMode_##gtkname##P=table_new(); \
    typedef GtkNaturalWrapMode(*sig_GtkNaturalWrapMode_##gtkname##P)(gtkname*);
#define TS_CALL_GtkNaturalWrapMode_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_GtkNaturalWrapMode_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s value:",nr,command,widget_id); \
          GtkNaturalWrapMode i_value = ((sig_GtkNaturalWrapMode_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %i\n",i_value); \
          fprintf(pargs->fpout, "%i\n", i_value); \
          fflush(pargs->fpout); \
      } else 

#define TS_DEF_PangoEllipsizeMode_gtknameP(gtkname) \
    FUS* table_PangoEllipsizeMode_##gtkname##P=table_new(); \
    typedef PangoEllipsizeMode(*sig_PangoEllipsizeMode_##gtkname##P)(gtkname*);
#define TS_CALL_PangoEllipsizeMode_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_PangoEllipsizeMode_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s value:",nr,command,widget_id); \
          PangoEllipsizeMode i_value = ((sig_PangoEllipsizeMode_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %i\n",i_value); \
          fprintf(pargs->fpout, "%i\n", i_value); \
          fflush(pargs->fpout); \
      } else 

#define TS_DEF_PangoWrapMode_gtknameP(gtkname) \
    FUS* table_PangoWrapMode_##gtkname##P=table_new(); \
    typedef PangoWrapMode(*sig_PangoWrapMode_##gtkname##P)(gtkname*);
#define TS_CALL_PangoWrapMode_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_PangoWrapMode_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s value:",nr,command,widget_id); \
          PangoWrapMode i_value = ((sig_PangoWrapMode_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %i\n",i_value); \
          fprintf(pargs->fpout, "%i\n", i_value); \
          fflush(pargs->fpout); \
      } else 

#define TS_DEF_void_gtknameP(gtkname) \
    FUS* table_void_##gtkname##P=table_new(); \
    typedef void(*sig_void_##gtkname##P)(gtkname*);
#define TS_CALL_void_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s\n",nr,command,widget_id); \
          ((sig_void_##gtkname##P)vu)(gtkcast(widget)); \
      } else 

#define TS_DEF_void_gtknameP_constcharP(gtkname) \
    FUS* table_void_##gtkname##P_constcharP=table_new(); \
    typedef void(*sig_void_##gtkname##P_constcharP)(gtkname*,const char*);
#define TS_CALL_void_gtknameP_constcharP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_constcharP,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          ((sig_void_##gtkname##P_constcharP)vu)(gtkcast(widget),operanda); \
      } else 

#define TS_DEF_void_gtknameP_double(gtkname) \
    FUS* table_void_##gtkname##P_double=table_new(); \
    typedef void(*sig_void_##gtkname##P_double)(gtkname*,double);
#define TS_CALL_void_gtknameP_double(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_double,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          double d = atof(operanda); \
          ((sig_void_##gtkname##P_double)vu)(gtkcast(widget),d); \
      } else 

#define TS_DEF_void_gtknameP_float(gtkname) \
    FUS* table_void_##gtkname##P_float=table_new(); \
    typedef void(*sig_void_##gtkname##P_float)(gtkname*,float);
#define TS_CALL_void_gtknameP_float(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_float,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          float f = atof(operanda); \
          ((sig_void_##gtkname##P_float)vu)(gtkcast(widget),f); \
      } else 

#define TS_DEF_void_gtknameP_gboolean(gtkname) \
    FUS* table_void_##gtkname##P_gboolean=table_new(); \
    typedef void(*sig_void_##gtkname##P_gboolean)(gtkname*,gboolean);
#define TS_CALL_void_gtknameP_gboolean(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_gboolean,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          gboolean i = atoi(operanda); \
          ((sig_void_##gtkname##P_gboolean)vu)(gtkcast(widget),i); \
      } else 

#define TS_DEF_void_gtknameP_guint(gtkname) \
    FUS* table_void_##gtkname##P_guint=table_new(); \
    typedef void(*sig_void_##gtkname##P_guint)(gtkname*,guint);
#define TS_CALL_void_gtknameP_guint(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_guint,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          guint i = atoi(operanda); \
          ((sig_void_##gtkname##P_guint)vu)(gtkcast(widget),i); \
      } else 

#define TS_DEF_void_gtknameP_int(gtkname) \
    FUS* table_void_##gtkname##P_int=table_new(); \
    typedef void(*sig_void_##gtkname##P_int)(gtkname*,int);
#define TS_CALL_void_gtknameP_int(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_int,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          int i = atoi(operanda); \
          ((sig_void_##gtkname##P_int)vu)(gtkcast(widget),i); \
      } else 

#define TS_DEF_void_gtknameP_GtkJustification(gtkname) \
    FUS* table_void_##gtkname##P_GtkJustification=table_new(); \
    typedef void(*sig_void_##gtkname##P_GtkJustification)(gtkname*,GtkJustification);
#define TS_CALL_void_gtknameP_GtkJustification(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_GtkJustification,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          GtkJustification i = atoi(operanda); \
          ((sig_void_##gtkname##P_GtkJustification)vu)(gtkcast(widget),i); \
      } else 

#define TS_DEF_void_gtknameP_GtkNaturalWrapMode(gtkname) \
    FUS* table_void_##gtkname##P_GtkNaturalWrapMode=table_new(); \
    typedef void(*sig_void_##gtkname##P_GtkNaturalWrapMode)(gtkname*,GtkNaturalWrapMode);
#define TS_CALL_void_gtknameP_GtkNaturalWrapMode(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_GtkNaturalWrapMode,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          GtkNaturalWrapMode i = atoi(operanda); \
          ((sig_void_##gtkname##P_GtkNaturalWrapMode)vu)(gtkcast(widget),i); \
      } else 

#define TS_DEF_void_gtknameP_PangoEllipsizeMode(gtkname) \
    FUS* table_void_##gtkname##P_PangoEllipsizeMode=table_new(); \
    typedef void(*sig_void_##gtkname##P_PangoEllipsizeMode)(gtkname*,PangoEllipsizeMode);
#define TS_CALL_void_gtknameP_PangoEllipsizeMode(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_PangoEllipsizeMode,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          PangoEllipsizeMode i = atoi(operanda); \
          ((sig_void_##gtkname##P_PangoEllipsizeMode)vu)(gtkcast(widget),i); \
      } else 

#define TS_DEF_void_gtknameP_PangoWrapMode(gtkname) \
    FUS* table_void_##gtkname##P_PangoWrapMode=table_new(); \
    typedef void(*sig_void_##gtkname##P_PangoWrapMode)(gtkname*,PangoWrapMode);
#define TS_CALL_void_gtknameP_PangoWrapMode(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_void_##gtkname##P_PangoWrapMode,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK%4i %s, widget_id: %s argument: %s\n",nr,command,widget_id,operanda); \
          PangoWrapMode i = atoi(operanda); \
          ((sig_void_##gtkname##P_PangoWrapMode)vu)(gtkcast(widget),i); \
      } else 
