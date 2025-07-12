#define __PAR_TO_VALUE(PAR_TYP,PAR_IS_GTK_WIDGET,PAR_FORMATFU,ARG,VALUE) \
        PAR_TYP VALUE; \
        if(PAR_IS_GTK_WIDGET) { \
            GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, ARG)); \
            VALUE = PAR_FORMATFU(widget); \
        } else { \
            VALUE = PAR_FORMATFU(ARG); \
        }
#define __RETOUT(RET_FORMAT) \
        if(DEBUG) fprintf(stderr,RET_FORMAT" (%s)\n",result,__func__); \
        fprintf(pargs->fpout, RET_FORMAT"\n", result); \
        fflush(pargs->fpout); 
//
//
#define SIGFU_DEF_RET(RET, RET_TYP, RET_FORMATFU, RET_FORMAT, RET_IS_GTK_WIDGET) \
    /* GtkTextDirection gtk_widget_get_default_direction() */ \
    /* SIGFU_DEF_RET(GtkTextDirection,GtkTextDirection,atoi,"%i",FALSE)*/ \
    typedef RET_TYP(* gtksig_##RET)(); \
    void wrapper_##RET(GtkBuilder* builder,char* COMMAND, char*, char*, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-0-0-0 %2$s() => ",__func__,COMMAND); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        RET_TYP result = ((gtksig_##RET)gtk_vu)(); \
        __RETOUT(RET_FORMAT) \
    }
#define SIGFU_CON_RET(COMMAND, RET) /*1-0-0-0*/ \
    /* SIGFU_CON_RET(gtk_widget_get_default_direction, GtkTextDirection) */ \
    table_add(#COMMAND,wrapper_##RET,COMMAND);
//
//
#define SIGFU_DEF_RET_PAR1(RET,  RET_TYP,  RET_FORMATFU,  RET_FORMAT,  RET_IS_GTK_WIDGET, \
                           PAR1, PAR1_TYP, PAR1_FORMATFU, PAR1_FORMAT, PAR1_IS_GTK_WIDGET)  \
    /* gboolean gtk_window_is_fullscreen (GtkWindow* window) */ \
    /* SIGFU_DEF_RET_PAR1(gboolean,gboolean,atoi,"%i",FALSE, */ \
    /*                    GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",TRUE) */ \
    typedef RET_TYP(* gtksig_##RET##_##PAR1)(PAR1_TYP); \
    void wrapper_##RET##_##PAR1(GtkBuilder* builder,char* COMMAND, char* arg1, char*, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-1-0-0 %2$s(%3$s) => ",__func__,COMMAND,arg1); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        RET_TYP result = ((gtksig_##RET##_##PAR1)gtk_vu)(value1); \
        __RETOUT(RET_FORMAT) \
    }
#define SIGFU_DEF_ARR_PAR1(RET,  RET_TYP,  RET_FORMATFU,  RET_FORMAT,  RET_IS_GTK_WIDGET, \
                           PAR1, PAR1_TYP, PAR1_FORMATFU, PAR1_FORMAT, PAR1_IS_GTK_WIDGET)  \
    /* e.g. char** gtk_widget_get_css_classes (GtkWidget* widget)*/ \
    /* SIGFU_DEF_ARR_PAR1(constcharPP,const char**,atoa,"%s",FALSE, */ \
    /*                    GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",TRUE) */ \
    typedef RET_TYP(* gtksig_##RET##_##PAR1)(PAR1_TYP); \
    void wrapper_##RET##_##PAR1(GtkBuilder* builder,char* COMMAND, char* arg1, char*, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-1-0-0 %2$s(%3$s) => ",__func__,COMMAND,arg1); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        RET_TYP result = ((gtksig_##RET##_##PAR1)gtk_vu)(value1); \
        while (*result != NULL) { \
            if(DEBUG) fprintf(stderr,RET_FORMAT" ",*result); \
            fprintf(pargs->fpout, RET_FORMAT" ", *result); \
            result++; \
        } \
        __RETOUT(RET_FORMAT) \
    }
#define SIGFU_CON_RET_PAR1(COMMAND,RET,PAR1) /*1-1-0-0*/ \
    /* SIGFU_CON_RET_PAR1(gtk_window_is_fullscreen, gboolean, GtkWindowP) */ \
    table_add(#COMMAND,wrapper_##RET##_##PAR1,COMMAND);
//
//
#define SIGFU_DEF_RET_PAR2_PAR1(RET,  RET_TYP,  RET_FORMATFU,  RET_FORMAT,  RET_IS_GTK_WIDGET, \
                                PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                                PAR1, PAR1_TYP, PAR1_FORMATFU, PAR1_FORMAT, PAR1_IS_GTK_WIDGET)  \
    /* gboolean gtk_widget_child_focus (GtkWidget* widget, GtkDirectionType direction) */ \
    /* SIGFU_DEF_RET_PAR2_PAR1(gboolean,gboolean,atoi,"%i",FALSE, */ \
    /*                           GtkDirectionType,GtkDirectionType,atoi,"%i",FALSE, */ \
    /*                           GtkWidgetP,GtkWidget*,GTK_WIDGET,"%p",TRUE) */ \
    typedef RET_TYP(* gtksig_##RET##_##PAR1##_##PAR2)(PAR1_TYP,PAR2_TYP); \
    void wrapper_##RET##_##PAR1##_##PAR2(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-1-1-0 %2$s(%3$s, %4$s) => ",__func__,COMMAND,arg1,arg2); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        RET_TYP result = ((gtksig_##RET##_##PAR1##_##PAR2)gtk_vu)(value1, value2); \
        __RETOUT(RET_FORMAT) \
    }
#define SIGFU_CON_RET_PAR1_PAR2(COMMAND,RET,PAR1,PAR2) /*1-1-1-0*/ \
    /* SIGFU_CON_RET_PAR1_PAR2(gtk_widget_child_focus,gboolean,GtkWidgetP,GtkDirectionType) */ \
    table_add(#COMMAND,wrapper_##RET##_##PAR1##_##PAR2,COMMAND);
//
//
#define SIGFU_DEF_RET_PAR2_PAR3_PAR1(RET,  RET_TYP,  RET_FORMATFU,  RET_FORMAT,  RET_IS_GTK_WIDGET, \
                                     PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                                     PAR3,PAR3_TYP,PAR3_FORMATFU,PAR3_FORMAT,PAR3_IS_GTK_WIDGET, \
                                     PAR1, PAR1_TYP, PAR1_FORMATFU, PAR1_FORMAT, PAR1_IS_GTK_WIDGET)  \
    typedef RET_TYP(* gtksig_##RET##_##PAR1##_##PAR2##_##PAR3)(PAR1_TYP,PAR2_TYP,PAR3_TYP); \
    void wrapper_##RET##_##PAR1##_##PAR2##_##PAR3(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char* arg3, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-1-1-1 %2$s(%3$s, %4$s, %5$s) => ",__func__,COMMAND,arg1,arg2,arg3); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        __PAR_TO_VALUE(PAR3_TYP,PAR3_IS_GTK_WIDGET,PAR3_FORMATFU,arg3,value3) \
        RET_TYP result = ((gtksig_##RET##_##PAR1##_##PAR2##_##PAR3)gtk_vu)(value1, value2, value3); \
        __RETOUT(RET_FORMAT) \
    }
#define SIGFU_CON_RET_PAR1_PAR2_PAR3(COMMAND,RET,PAR1,PAR2,PAR3) /*1-1-1-1*/ \
    table_add(#COMMAND,wrapper_##RET##_##PAR1##_##PAR2##_##PAR3,COMMAND);
//
//
#define SIGFU_DEF_RET_PAR2_PAR3_PAR4_PAR1( \
                RET,  RET_TYP,  RET_FORMATFU,  RET_FORMAT,  RET_IS_GTK_WIDGET, \
                PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                PAR3,PAR3_TYP,PAR3_FORMATFU,PAR3_FORMAT,PAR3_IS_GTK_WIDGET, \
                PAR4,PAR4_TYP,PAR4_FORMATFU,PAR4_FORMAT,PAR4_IS_GTK_WIDGET, \
                PAR1, PAR1_TYP, PAR1_FORMATFU, PAR1_FORMAT, PAR1_IS_GTK_WIDGET)  \
    typedef RET_TYP(* gtksig_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4)(PAR1_TYP,PAR2_TYP,PAR3_TYP,PAR4_TYP); \
    void wrapper_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char* arg3, char* arg4, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-1-1-1-1 %2$s(%3$s, %4$s, %5$s, %6$s) => ",__func__,COMMAND,arg1,arg2,arg3,arg4); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        __PAR_TO_VALUE(PAR3_TYP,PAR3_IS_GTK_WIDGET,PAR3_FORMATFU,arg3,value3) \
        __PAR_TO_VALUE(PAR4_TYP,PAR4_IS_GTK_WIDGET,PAR4_FORMATFU,arg4,value4) \
        RET_TYP result = ((gtksig_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4)gtk_vu)(value1, value2, value3, value4); \
        __RETOUT(RET_FORMAT) \
    }
#define SIGFU_CON_RET_PAR1_PAR2_PAR3_PAR4(COMMAND,RET,PAR1,PAR2,PAR3,PAR4) /*1-1-1-1-1*/ \
    table_add(#COMMAND,wrapper_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4,COMMAND);
//
//
#define SIGFU_DEF_RET_PAR2_PAR3_PAR4_PAR5_PAR1( \
                RET,  RET_TYP,  RET_FORMATFU,  RET_FORMAT,  RET_IS_GTK_WIDGET, \
                PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                PAR3,PAR3_TYP,PAR3_FORMATFU,PAR3_FORMAT,PAR3_IS_GTK_WIDGET, \
                PAR4,PAR4_TYP,PAR4_FORMATFU,PAR4_FORMAT,PAR4_IS_GTK_WIDGET, \
                PAR5,PAR5_TYP,PAR5_FORMATFU,PAR5_FORMAT,PAR5_IS_GTK_WIDGET, \
                PAR1, PAR1_TYP, PAR1_FORMATFU, PAR1_FORMAT, PAR1_IS_GTK_WIDGET)  \
    typedef RET_TYP(* gtksig_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5)(PAR1_TYP,PAR2_TYP,PAR3_TYP,PAR4_TYP,PAR5_TYP); \
    void wrapper_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char* arg3, char* arg4, char* arg5, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-1-1-1-1-1 %2$s(%3$s, %4$s, %5$s, %6$s, %7$s) => ",__func__,COMMAND,arg1,arg2,arg3,arg4,arg5); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        __PAR_TO_VALUE(PAR3_TYP,PAR3_IS_GTK_WIDGET,PAR3_FORMATFU,arg3,value3) \
        __PAR_TO_VALUE(PAR4_TYP,PAR4_IS_GTK_WIDGET,PAR4_FORMATFU,arg4,value4) \
        __PAR_TO_VALUE(PAR5_TYP,PAR5_IS_GTK_WIDGET,PAR5_FORMATFU,arg5,value5) \
        RET_TYP result = ((gtksig_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5)gtk_vu)(value1, value2, value3, value4, value5); \
        __RETOUT(RET_FORMAT) \
    }
#define SIGFU_CON_RET_PAR1_PAR2_PAR3_PAR4_PAR5(COMMAND,RET,PAR1,PAR2,PAR3,PAR4,PAR5) /*1-1-1-1-1-1*/ \
    table_add(#COMMAND,wrapper_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5,COMMAND);
//
//
#define SIGFU_DEF_RET_PAR2_PAR3_PAR4_PAR5_PAR6_PAR1( \
                RET,  RET_TYP,  RET_FORMATFU,  RET_FORMAT,  RET_IS_GTK_WIDGET, \
                PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                PAR3,PAR3_TYP,PAR3_FORMATFU,PAR3_FORMAT,PAR3_IS_GTK_WIDGET, \
                PAR4,PAR4_TYP,PAR4_FORMATFU,PAR4_FORMAT,PAR4_IS_GTK_WIDGET, \
                PAR5,PAR5_TYP,PAR5_FORMATFU,PAR5_FORMAT,PAR5_IS_GTK_WIDGET, \
                PAR6,PAR6_TYP,PAR6_FORMATFU,PAR6_FORMAT,PAR6_IS_GTK_WIDGET, \
                PAR1, PAR1_TYP, PAR1_FORMATFU, PAR1_FORMAT, PAR1_IS_GTK_WIDGET)  \
    typedef RET_TYP(* gtksig_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5##_##PAR6)(PAR1_TYP,PAR2_TYP,PAR3_TYP,PAR4_TYP,PAR5_TYP,PAR6_TYP); \
    void wrapper_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5##_##PAR6(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char* arg3, char* arg4, char* arg5, char* arg6) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 1-1-1-1-1-1-1 %2$s(%3$s, %4$s, %5$s, %6$s, %7$s, %8$s) => ",__func__,COMMAND,arg1,arg2,arg3,arg4,arg5,arg6); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        __PAR_TO_VALUE(PAR3_TYP,PAR3_IS_GTK_WIDGET,PAR3_FORMATFU,arg3,value3) \
        __PAR_TO_VALUE(PAR4_TYP,PAR4_IS_GTK_WIDGET,PAR4_FORMATFU,arg4,value4) \
        __PAR_TO_VALUE(PAR5_TYP,PAR5_IS_GTK_WIDGET,PAR5_FORMATFU,arg5,value5) \
        __PAR_TO_VALUE(PAR6_TYP,PAR6_IS_GTK_WIDGET,PAR6_FORMATFU,arg6,value6) \
        RET_TYP result = ((gtksig_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5##_##PAR6)gtk_vu)(value1, value2, value3, value4, value5, value6); \
        __RETOUT(RET_FORMAT) \
    }
#define SIGFU_CON_RET_PAR1_PAR2_PAR3_PAR4_PAR5_PAR6(COMMAND,RET,PAR1,PAR2,PAR3,PAR4,PAR5,PAR6) /*1-1-1-1-1-1-1*/ \
    table_add(#COMMAND,wrapper_##RET##_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5_##PAR6,COMMAND);
//
//
#define SIGFU_DEF_VOID_PAR1(PAR1, PAR1_TYP, PAR1_FORMATFU, PAR1_FORMAT, PAR1_IS_GTK_WIDGET) \
    /* void gtk_window_set_auto_startup_notification(gboolean) */ \
    /* SIGFU_DEF_VOID_PAR1(gboolean,gboolean,atoi,"%i",FALSE) */ \
    /* void gtk_window_maximize(GtkWindow* window) */ \
    /* SIGFU_DEF_VOID_PAR1(GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",TRUE) */ \
    typedef void(* gtksig_void_##PAR1)(PAR1_TYP); \
    void wrapper_void_##PAR1(GtkBuilder* builder,char* COMMAND, char* arg1, char*, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 0-1-0-0 %2$s(%3$s) (%1$s)\n",__func__,COMMAND,arg1); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        ((gtksig_void_##PAR1)gtk_vu)(value1); \
    }
#define SIGFU_CON_VOID_PAR1(COMMAND,PAR1) /*0-1-0-0*/ \
    /* SIGFU_CON_VOID_PAR1(gtk_window_set_auto_startup_notification, gboolean) */ \
    /* SIGFU_CON_VOID_PAR1(gtk_window_maximize, GtkWindowP) */ \
    table_add(#COMMAND,wrapper_void_##PAR1,COMMAND);
//
//
#define SIGFU_DEF_VOID_PAR2_PAR1(PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                                 PAR1,PAR1_TYP,PAR1_FORMATFU,PAR1_FORMAT,PAR1_IS_GTK_WIDGET) \
    /* SIGFU_DEF_VOID_PAR2_PAR1(constcharP,const char*,atoa,"%s",FALSE, */ \
    /* GtkWindowP,GtkWindow*,GTK_WINDOW,"%p",TRUE) */ \
    typedef void(* gtksig_void_##PAR1##_##PAR2)(PAR1_TYP,PAR2_TYP); \
    void wrapper_void_##PAR1##_##PAR2(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char*, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 0-1-1-0 %2$s(%3$s, %4$s) (%1$s)\n",__func__,COMMAND,arg1,arg2); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        ((gtksig_void_##PAR1##_##PAR2)gtk_vu)(value1,value2); \
    }
#define SIGFU_CON_VOID_PAR1_PAR2(COMMAND,PAR1,PAR2) /*0-1-1-0*/ \
    /* SIGFU_CON_VOID_gtkclassP_PAR2(gtk_window_set_title, GtkWindowP, constcharP) */ \
    table_add(#COMMAND,wrapper_void_##PAR1##_##PAR2,COMMAND);
//
//
#define SIGFU_DEF_VOID_PAR2_PAR3_PAR1(PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                                      PAR3,PAR3_TYP,PAR3_FORMATFU,PAR3_FORMAT,PAR3_IS_GTK_WIDGET, \
                                      PAR1,PAR1_TYP,PAR1_FORMATFU,PAR1_FORMAT,PAR1_IS_GTK_WIDGET) \
    typedef void(* gtksig_void_##PAR1##_##PAR2##_##PAR3)(PAR1_TYP,PAR2_TYP,PAR3_TYP); \
    void wrapper_void_##PAR1##_##PAR2##_##PAR3(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char* arg3, char*, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 0-1-1-1 %2$s(%3$s, %4$s, %5$s) (%1$s)\n",__func__,COMMAND,arg1,arg2,arg3); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        __PAR_TO_VALUE(PAR3_TYP,PAR3_IS_GTK_WIDGET,PAR3_FORMATFU,arg3,value3) \
        ((gtksig_void_##PAR1##_##PAR2##_##PAR3)gtk_vu)(value1,value2,value3); \
    }
#define SIGFU_CON_VOID_PAR1_PAR2_PAR3(COMMAND,PAR1,PAR2,PAR3) /*0-1-1-1*/ \
    table_add(#COMMAND,wrapper_void_##PAR1##_##PAR2##_##PAR3,COMMAND);
//
//
#define SIGFU_DEF_VOID_PAR2_PAR3_PAR4_PAR1(PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                                           PAR3,PAR3_TYP,PAR3_FORMATFU,PAR3_FORMAT,PAR3_IS_GTK_WIDGET, \
                                           PAR4,PAR4_TYP,PAR4_FORMATFU,PAR4_FORMAT,PAR4_IS_GTK_WIDGET, \
                                           PAR1,PAR1_TYP,PAR1_FORMATFU,PAR1_FORMAT,PAR1_IS_GTK_WIDGET) \
    typedef void(* gtksig_void_##PAR1##_##PAR2##_##PAR3##_##PAR4)(PAR1_TYP,PAR2_TYP,PAR3_TYP,PAR4_TYP); \
    void wrapper_void_##PAR1##_##PAR2##_##PAR3##_##PAR4(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char* arg3, char* arg4, char*, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 0-1-1-1-1 %2$s(%3$s, %4$s, %5$s, %6$s) (%1$s)\n",__func__,COMMAND,arg1,arg2,arg3,arg4); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        __PAR_TO_VALUE(PAR3_TYP,PAR3_IS_GTK_WIDGET,PAR3_FORMATFU,arg3,value3) \
        __PAR_TO_VALUE(PAR4_TYP,PAR4_IS_GTK_WIDGET,PAR4_FORMATFU,arg4,value4) \
        ((gtksig_void_##PAR1##_##PAR2##_##PAR3##_##PAR4)gtk_vu)(value1,value2,value3,value4); \
    }
#define SIGFU_CON_VOID_PAR1_PAR2_PAR3_PAR4(COMMAND,PAR1,PAR2,PAR3,PAR4) /*0-1-1-1-1*/ \
    table_add(#COMMAND,wrapper_void_##PAR1##_##PAR2##_##PAR3##_##PAR4,COMMAND);
//
//
#define SIGFU_DEF_VOID_PAR2_PAR3_PAR4_PAR5_PAR1( \
                PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                PAR3,PAR3_TYP,PAR3_FORMATFU,PAR3_FORMAT,PAR3_IS_GTK_WIDGET, \
                PAR4,PAR4_TYP,PAR4_FORMATFU,PAR4_FORMAT,PAR4_IS_GTK_WIDGET, \
                PAR5,PAR5_TYP,PAR5_FORMATFU,PAR5_FORMAT,PAR5_IS_GTK_WIDGET, \
                PAR1,PAR1_TYP,PAR1_FORMATFU,PAR1_FORMAT,PAR1_IS_GTK_WIDGET) \
    typedef void(* gtksig_void_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5)(PAR1_TYP,PAR2_TYP,PAR3_TYP,PAR4_TYP,PAR5_TYP); \
    void wrapper_void_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char* arg3, char* arg4, char* arg5, char*) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 0-1-1-1-1-1 %2$s(%3$s, %4$s, %5$s, %6$s, %7$s) (%1$s)\n",__func__,COMMAND,arg1,arg2,arg3,arg4,arg5); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        __PAR_TO_VALUE(PAR3_TYP,PAR3_IS_GTK_WIDGET,PAR3_FORMATFU,arg3,value3) \
        __PAR_TO_VALUE(PAR4_TYP,PAR4_IS_GTK_WIDGET,PAR4_FORMATFU,arg4,value4) \
        __PAR_TO_VALUE(PAR5_TYP,PAR5_IS_GTK_WIDGET,PAR5_FORMATFU,arg5,value5) \
        ((gtksig_void_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5)gtk_vu)(value1,value2,value3,value4,value5); \
    }
#define SIGFU_CON_VOID_PAR1_PAR2_PAR3_PAR4_PAR5(COMMAND,PAR1,PAR2,PAR3,PAR4,PAR5) /*0-1-1-1-1-1*/ \
    table_add(#COMMAND,wrapper_void_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5,COMMAND);
//
//
#define SIGFU_DEF_VOID_PAR2_PAR3_PAR4_PAR5_PAR6_PAR1( \
                PAR2,PAR2_TYP,PAR2_FORMATFU,PAR2_FORMAT,PAR2_IS_GTK_WIDGET, \
                PAR3,PAR3_TYP,PAR3_FORMATFU,PAR3_FORMAT,PAR3_IS_GTK_WIDGET, \
                PAR4,PAR4_TYP,PAR4_FORMATFU,PAR4_FORMAT,PAR4_IS_GTK_WIDGET, \
                PAR5,PAR5_TYP,PAR5_FORMATFU,PAR5_FORMAT,PAR5_IS_GTK_WIDGET, \
                PAR6,PAR6_TYP,PAR6_FORMATFU,PAR6_FORMAT,PAR6_IS_GTK_WIDGET, \
                PAR1,PAR1_TYP,PAR1_FORMATFU,PAR1_FORMAT,PAR1_IS_GTK_WIDGET) \
    typedef void(* gtksig_void_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5##_##PAR6)(PAR1_TYP,PAR2_TYP,PAR3_TYP,PAR4_TYP,PAR5_TYP,PAR6_TYP); \
    void wrapper_void_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5##_##PAR6(GtkBuilder* builder,char* COMMAND, char* arg1, char* arg2, char* arg3, char* arg4, char* arg5, char* arg6) { \
        if(DEBUG) fprintf(stderr,"GTKCALL 0-1-1-1-1-1 %2$s(%3$s, %4$s, %5$s, %6$s, %7$s, %8$s) (%1$s)\n",__func__,COMMAND,arg1,arg2,arg3,arg4,arg5,arg6); \
        void* gtk_vu=table_get_gtk_fu(COMMAND); \
        __PAR_TO_VALUE(PAR1_TYP,PAR1_IS_GTK_WIDGET,PAR1_FORMATFU,arg1,value1) \
        __PAR_TO_VALUE(PAR2_TYP,PAR2_IS_GTK_WIDGET,PAR2_FORMATFU,arg2,value2) \
        __PAR_TO_VALUE(PAR3_TYP,PAR3_IS_GTK_WIDGET,PAR3_FORMATFU,arg3,value3) \
        __PAR_TO_VALUE(PAR4_TYP,PAR4_IS_GTK_WIDGET,PAR4_FORMATFU,arg4,value4) \
        __PAR_TO_VALUE(PAR5_TYP,PAR5_IS_GTK_WIDGET,PAR5_FORMATFU,arg5,value5) \
        __PAR_TO_VALUE(PAR6_TYP,PAR6_IS_GTK_WIDGET,PAR6_FORMATFU,arg6,value6) \
        ((gtksig_void_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5##_##PAR6)gtk_vu)(value1,value2,value3,value4,value5,value6); \
    }
#define SIGFU_CON_VOID_PAR1_PAR2_PAR3_PAR4_PAR5_PAR6(COMMAND,PAR1,PAR2,PAR3,PAR4,PAR5,PAR6) /*0-1-1-1-1-1-1*/ \
    table_add(#COMMAND,wrapper_void_##PAR1##_##PAR2##_##PAR3##_##PAR4##_##PAR5##_##PAR6,COMMAND);



