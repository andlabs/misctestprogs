// 22 july 2015
#include <gtk/gtk.h>

int main(void)
{
	GtkWidget *fcd;
	GtkFileChooser *fc;

	gtk_init(NULL, NULL);
	fcd = gtk_file_chooser_dialog_new(NULL, NULL, GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	fc = GTK_FILE_CHOOSER(fcd);
	gtk_file_chooser_set_local_only(fc, FALSE);
	gtk_file_chooser_set_select_multiple(fc, FALSE);
	gtk_file_chooser_set_show_hidden(fc, TRUE);
	gtk_file_chooser_set_do_overwrite_confirmation(fc, TRUE);
	gtk_file_chooser_set_create_folders(fc, TRUE);
	gtk_dialog_run(GTK_DIALOG(fcd));
	gtk_widget_destroy(fcd);
	return 0;
}
