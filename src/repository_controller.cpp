#include "repository_controller.h"

repository_controller::repository_controller(std::string &dir) :
	repo(dir.c_str()),
	refs(repo),
	prefs(),
	clist(refs, repo, prefs)
{}

void repository_controller::display_commits(std::function<void(const char *, size_t)> display_line)
{
	while (!clist.empty()) {
		struct commit_graph_info graph;
		git::commit commit = clist.get_next_commit(graph);
		display_line(commit.summary(), strlen(commit.summary()));
	}
}
