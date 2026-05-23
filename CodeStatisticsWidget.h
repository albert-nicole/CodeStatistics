#pragma once
#include <QWidget>

class CodeStatisticsWidget : public QWidget
{
	Q_OBJECT
public:
	using uint64_t = unsigned long long;
	CodeStatisticsWidget(QWidget* parent = nullptr);
	~CodeStatisticsWidget() {}

private:
	void calculateStatistics();

private slots:
	void onOpenDirectoryClicked();
	void onClearClicked();
	void onAddFilterClicked();
};
