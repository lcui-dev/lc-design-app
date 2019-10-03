#include <LCUI.h>
#include <LCUI/gui/widget.h>
#include <LCUI/gui/widget/textview.h>
#include <LCUI/gui/widget/textedit.h>
#include <LCUI/timer.h>
#include "home.h"
#include "store.h"

typedef struct HomeViewRec_ {
	LCUI_Widget input;
	LCUI_Widget feedback;

	struct store store;
	int feedback_timer;
} HomeViewRec, *HomeView;

LCUI_WidgetPrototype home_view_proto;

static void OnFeedbackTimeout(void *arg)
{
	HomeView self;

	self = Widget_GetData(arg, home_view_proto);
	Widget_Hide(self->feedback);
	self->feedback_timer = 0;
}

static void OnBtnSaveClick(LCUI_Widget w, LCUI_WidgetEvent e, void *unused)
{
	size_t len;
	wchar_t wcs[256];

	HomeView self;

	self = Widget_GetData(e->data, home_view_proto);
	len = TextEdit_GetTextW(self->input, 0, 255, wcs);
	wcs[len] = 0;

	len = LCUI_EncodeUTF8String(self->store.message, wcs, 255);
	self->store.message[len] = 0;
	store_save(&self->store);
	if (self->feedback_timer) {
		LCUITimer_Free(self->feedback_timer);
	}
	Widget_Show(self->feedback);
	self->feedback_timer = LCUI_SetTimeout(2000, OnFeedbackTimeout, e->data);
}

static void HomeView_OnReady(LCUI_Widget w, LCUI_WidgetEvent e, void *arg)
{
	Dict *dict;
	HomeView self;
	LCUI_Widget btn;

	self = Widget_GetData(w, home_view_proto);
	if (store_load(&self->store) != 0) {
		store_init(&self->store);
		store_save(&self->store);
	}
	dict = Widget_CollectReferences(w);
	btn = Dict_FetchValue(dict, "btn-save-message");
	self->input = Dict_FetchValue(dict, "input-message");
	self->feedback = Dict_FetchValue(dict, "feedback");
	Widget_BindEvent(btn, "click", OnBtnSaveClick, w, NULL);
	TextEdit_SetText(self->input, self->store.message);
	Widget_Hide(self->feedback);
	Widget_UnbindEvent(w, "ready", HomeView_OnReady);
	Dict_Release(dict);
}

static void HomeView_OnInit(LCUI_Widget w)
{
	Widget_AddClass(w, "view v-home");
	Widget_AddData(w, home_view_proto, sizeof(HomeViewRec));
	Widget_BindEvent(w, "ready", HomeView_OnReady, NULL, NULL);
}

void UI_InitHomeView(void)
{
	home_view_proto = LCUIWidget_NewPrototype("home", NULL);
	home_view_proto->init = HomeView_OnInit;
}
