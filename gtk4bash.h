#define SIGFU_DEF_RET(RET,TYP,FORMAT) /* e.g. RET gtk_widget_get_default_direction */ \
    /* GtkTextDirection gtk_widget_get_default_direction() */ \
    /* SIGFU_DEF_RET(GtkTextDirection,GtkTextDirection,"%i")*/ \
    typedef TYP(* sig_##RET)(); \
    void sigfu_##RET(GtkBuilder* builder,char* COMMAND, char*, char*, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-0-0 %2$s() => ",__func__,COMMAND); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        TYP result = ((sig_##RET)gtk_vu)(); \
        if(DEBUG) fprintf(stderr,FORMAT" (%s)\n",result,__func__); \
        fprintf(pargs->fpout, FORMAT"\n", result); \
        fflush(pargs->fpout); \
    }
#define SIGFU_CON_RET(RET,COMMAND) \
    /* SIGFU_CON_RET(GtkTextDirection,gtk_widget_get_default_direction) */ \
    table_add(#COMMAND,sigfu_##RET,COMMAND);
//
//
#define SIGFU_DEF_RET_gtknameP(RET,TYP,FORMAT,GTKNAME,GTKCAST) /* e.g. gtk_window_is_fullscreen */ \
    typedef TYP(* sig_##RET##_##GTKNAME##P)(GTKNAME*); \
    void sigfu_##RET##_##GTKNAME##P(GtkBuilder* builder,char* COMMAND, char* arg1, char*, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-1-0 %2$s(%3$s) => ",__func__,COMMAND, arg1); \
        GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, arg1)); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        TYP result = ((sig_##RET##_##GTKNAME##P)gtk_vu)(GTKCAST(widget)); \
        if(DEBUG) fprintf(stderr,FORMAT" (%s)\n",result,__func__); \
        fprintf(pargs->fpout, FORMAT"\n", result); \
        fflush(pargs->fpout); \
    }
#define SIGFU_CON_RET_gtknameP(RET,GTKNAME,COMMAND) \
    table_add(#COMMAND,sigfu_##RET##_##GTKNAME##P,COMMAND);
//
//
#define SIGFU_DEF_VOID_PAR1(PAR1,TYP1,FORMATFU) /* e.g. gtk_window_set_auto_startup_notification PAR1 */ \
    /* void gtk_window_set_auto_startup_notification(gboolean) */ \
    /* SIGFU_DEF_VOID_PAR1(gboolean,gboolean,atoi) */ \
    typedef void(* sig_void_##PAR1)(TYP1); \
    void sigfu_void_##PAR1(GtkBuilder* builder,char* COMMAND, char* arg1, char*, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 0-1-0a %2$s(%3$s) (%1$s)\n",__func__,COMMAND,arg1); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        TYP1 value = FORMATFU(arg1); \
        ((sig_void_##PAR1)gtk_vu)(value); \
    }
#define SIGFU_CON_VOID_PAR1(PAR1,COMMAND) \
    /* SIGFU_CON_VOID_PAR1(gboolean,gtk_window_set_auto_startup_notification) */ \
    table_add(#COMMAND,sigfu_void_##PAR1,COMMAND);
//
//
#define SIGFU_DEF_VOID_gtknameP(PAR2,GTKNAME,GTKCAST) /* e.g. gtk_window_maximize */ \
    typedef void(* sig_void_##GTKNAME##P_##PAR2)(GTKNAME*); \
    void sigfu_void_##GTKNAME##P_##PAR2(GtkBuilder* builder,char* COMMAND, char* arg1, char*, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 0-1-0b %2$s(%3$s) (%1$s)\n",__func__,COMMAND,arg1); \
        GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, arg1)); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        ((sig_void_##GTKNAME##P_##PAR2)gtk_vu)(GTKCAST(widget)); \
    }
#define SIGFU_CON_VOID_gtknameP(PAR2,GTKNAME,COMMAND) \
    table_add(#COMMAND,sigfu_void_##GTKNAME##P_##PAR2,COMMAND);
//
//
#define SIGFU_DEF_VOID_gtknameP_PAR2(PAR2,TYP2,FORMATFU,GTKNAME,GTKCAST) /* e.g. gtk_window_set_title */ \
    typedef void(* sig_void_##GTKNAME##P_##PAR2)(GTKNAME*,TYP2); \
    void sigfu_void_##GTKNAME##P_##PAR2(GtkBuilder* builder,char* COMMAND, char* arg1, char*arg2, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 0-1-1 %2$s(%3$s, %4$s) (%1$s)\n",__func__,COMMAND,arg1,arg2); \
        GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, arg1)); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        TYP2 value = FORMATFU(arg2); \
        ((sig_void_##GTKNAME##P_##PAR2)gtk_vu)(GTKCAST(widget),value); \
    }
#define SIGFU_CON_VOID_gtknameP_PAR2(PAR2,GTKNAME,COMMAND) \
    table_add(#COMMAND,sigfu_void_##GTKNAME##P_##PAR2,COMMAND);
