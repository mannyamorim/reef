#include "repository_controller.h"

repository_controller::repository_controller(std::string &dir) :
	repo(dir.c_str()),
	refs(repo),
	prefs(),
	clist(refs, repo, prefs)
{}

void repository_controller::display_commits(std::function<void(QString &)> display_line)
{
	while (!clist.empty()) {
		struct commit_graph_info graph;
		git::commit commit = clist.get_next_commit(graph);
		QString summary = QString::fromUtf8(commit.summary());
		display_line(summary);
	}
}
