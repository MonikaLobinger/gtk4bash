#define TS_DEF_constcharP_gtknameP(gtkname) \
    FUS* table_constcharP_##gtkname##P=table_new(); \
    typedef const char*(*sig_constcharP_##gtkname##P)(gtkname*);
#ifdef _________________________________________________________KEINE_ERKLAERUNG
    TS_DEF_constcharP_gtknameP(GtkWidgetClass)
    //evaluiert zu:
    FUS* table_constcharP_GtkWidgetClassP=table_new();
    typedef const char*(*sig_constcharP_GtkWidgetClassP)(GtkWidgetClass*);
    //kann so verwendet werden:
    table_add(table_constcharP_GtkWidgetClassP, "gtk_widget_class_get_css_name",gtk_widget_class_get_css_name);
#endif // _________________________________________________________

#define TS_CALL_constcharP_gtknameP(gtkname,gtkcast,nr) \
      if(NULL != (vu=table_get(table_constcharP_##gtkname##P,command))) { \
          if(DEBUG) fprintf(stderr,"CALLBACK##nr %s, widget_id: %s value:",command,widget_id); \
          const char* value = ((sig_constcharP_##gtkname##P)vu)(gtkcast(widget)); \
          if(DEBUG) fprintf(stderr," %s\n",value); \
          fprintf(pargs->fpout, "%s\n", value); \
          fflush(pargs->fpout); \
      } else 
#ifdef _________________________________________________________KEINE_ERKLAERUNG
    TS_CALL_constcharP_gtknameP(GtkWidgetClass,GTK_WIDGET_GET_CLASS,004)
    //evaluiert zu:
    if(NULL != (vu=table_get(table_constcharP_GtkWidgetClassP,command))) {
        if(DEBUG) fprintf(stderr,"CALLBACK004 %s, widget_id: %s value:",command,widget_id);
        const char* value = ((sig_constcharP_GtkWidgetClassP)vu)(GTK_WIDGET_GET_CLASS(widget));
        if(DEBUG) fprintf(stderr," %s\n",value);
        fprintf(pargs->fpout, "%s\n", value);
        fflush(pargs->fpout);
    } else
#endif // _________________________________________________________
