#include "CodeStatisticsWidget.h"
#include <QString>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QHeaderView>
#include <QDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QTableWidgetItem>

CodeStatisticsWidget::CodeStatisticsWidget(QWidget* parent)
{
	this->setWindowTitle("Code Statistics");
	this->setMinimumWidth(1000);
	this->setMinimumHeight(700);

	this->setStyleSheet(
		"QLabel, QLineEdit, QPushButton {"
		"   min-height: 35px;"
		"}"
	);

	QTableWidget* table = new QTableWidget(this);
	QHeaderView* header = table->horizontalHeader();
	//header->setSectionResizeMode(QHeaderView::Interactive);
	header->setSectionResizeMode(QHeaderView::Stretch);
	table->setObjectName("tableWidget");
	table->clear();

	int colCount = 9;
	table->setColumnCount(colCount);
	int colWidth = this->geometry().width() / colCount;
	for (int i = 0; i < colCount; ++i)
	{
		table->setColumnWidth(i, colWidth);
	}
	table->setHorizontalHeaderLabels({ "文件名", "类型", "行数", "代码行数", "注释行数", "空白行数","字符数", "文件大小", "路径"});
	table->setRowCount(0);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers); // 设置表格为只读模式

	QLabel* fileCountLabel = new QLabel("文件数:", this);
	QLineEdit* fileCountEdit = new QLineEdit(this);
	fileCountEdit->setObjectName("fileCountEdit");
	fileCountEdit->setReadOnly(true);
	QHBoxLayout* fileCountLayout = new QHBoxLayout();
	fileCountLayout->addWidget(fileCountLabel);
	fileCountLayout->addWidget(fileCountEdit);

	QLabel* codeLinesLabel = new QLabel("代码行数:", this);
	QLineEdit* codeLinesEdit = new QLineEdit(this);
	codeLinesEdit->setObjectName("codeLinesEdit");
	codeLinesEdit->setReadOnly(true);
	QHBoxLayout* codeLinesLayout = new QHBoxLayout();
	codeLinesLayout->addWidget(codeLinesLabel);
	codeLinesLayout->addWidget(codeLinesEdit);

	QLabel* dirPathLabel = new QLabel("目录:", this);
	QLineEdit* dirPathEdit = new QLineEdit(this);
	dirPathEdit->setObjectName("dirPathEdit");
	dirPathEdit->setReadOnly(true);
	QHBoxLayout* dirPathLayout = new QHBoxLayout();
	dirPathLayout->addWidget(dirPathLabel);
	dirPathLayout->addWidget(dirPathEdit);

	QPushButton* openDirButton = new QPushButton("打开目录", this);

	QLabel* bytesCountLabel = new QLabel("字节数:", this);
	QLineEdit* bytesCountEdit = new QLineEdit(this);
	bytesCountEdit->setObjectName("bytesCountEdit");
	bytesCountEdit->setReadOnly(true);
	QHBoxLayout* bytesCountLayout = new QHBoxLayout();
	bytesCountLayout->addWidget(bytesCountLabel);
	bytesCountLayout->addWidget(bytesCountEdit);

	QLabel* commentLinesLabel = new QLabel("注释行数:", this);
	QLineEdit* commentLinesEdit = new QLineEdit(this);
	commentLinesEdit->setObjectName("commentLinesEdit");
	commentLinesEdit->setReadOnly(true);
	QHBoxLayout* commentLinesLayout = new QHBoxLayout();
	commentLinesLayout->addWidget(commentLinesLabel);
	commentLinesLayout->addWidget(commentLinesEdit);

	QLabel* filterLabel = new QLabel("过滤:", this);
	QLineEdit* filterEdit = new QLineEdit(this);
	filterEdit->setObjectName("filterEdit");
	filterEdit->setReadOnly(true);
	filterEdit->setPlaceholderText("文件后缀名,如: *.cpp;*.h");
	QHBoxLayout* filterLayout = new QHBoxLayout();
	filterLayout->addWidget(filterLabel);
	filterLayout->addWidget(filterEdit);

	QPushButton* clearButton = new QPushButton("清空", this);
	QPushButton* addButton = new QPushButton("添加", this);

	QLabel* totalLinesLabel = new QLabel("总行数:", this);
	QLineEdit* totalLinesEdit = new QLineEdit(this);
	totalLinesEdit->setObjectName("totalLinesEdit");
	totalLinesEdit->setReadOnly(true);
	QHBoxLayout* totalLinesLayout = new QHBoxLayout();
	totalLinesLayout->addWidget(totalLinesLabel);
	totalLinesLayout->addWidget(totalLinesEdit);

	QLabel* blankLinesLabel = new QLabel("空白行数:", this);
	QLineEdit* blankLinesEdit = new QLineEdit(this);
	blankLinesEdit->setObjectName("blankLinesEdit");
	blankLinesEdit->setReadOnly(true);
	QHBoxLayout* blankLinesLayout = new QHBoxLayout();
	blankLinesLayout->addWidget(blankLinesLabel);
	blankLinesLayout->addWidget(blankLinesEdit);

	QPushButton* startButton = new QPushButton("开始", this);

	QVBoxLayout* firstColLayout = new QVBoxLayout();
	firstColLayout->addLayout(fileCountLayout);
	firstColLayout->addLayout(bytesCountLayout);
	firstColLayout->addLayout(totalLinesLayout);

	QVBoxLayout* secondColLayout = new QVBoxLayout();
	secondColLayout->addLayout(codeLinesLayout);
	secondColLayout->addLayout(commentLinesLayout);
	secondColLayout->addLayout(blankLinesLayout);

	QVBoxLayout* thirdColLayout = new QVBoxLayout();
	thirdColLayout->addLayout(dirPathLayout);
	thirdColLayout->addLayout(filterLayout);
	thirdColLayout->addWidget(startButton);

	QVBoxLayout* fourthColLayout = new QVBoxLayout();
	fourthColLayout->addWidget(openDirButton);
	fourthColLayout->addWidget(addButton);
	fourthColLayout->addWidget(clearButton);

	QHBoxLayout* horizontalLayout = new QHBoxLayout();
	horizontalLayout->addLayout(firstColLayout);
	horizontalLayout->addLayout(secondColLayout);
	horizontalLayout->addLayout(thirdColLayout);
	horizontalLayout->addLayout(fourthColLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(table);
	mainLayout->addLayout(horizontalLayout);

	connect(openDirButton, &QPushButton::clicked, this, &CodeStatisticsWidget::onOpenDirectoryClicked);
	connect(clearButton, &QPushButton::clicked, this, &CodeStatisticsWidget::onClearClicked);
	connect(startButton, &QPushButton::clicked, this, &CodeStatisticsWidget::calculateStatistics);
	connect(addButton, &QPushButton::clicked, this, &CodeStatisticsWidget::onAddFilterClicked);


	connect(table, &QTableWidget::cellClicked, this, [=](int row, int column) {

		//用户点击的是 相对路径 这一列触发打开目录
		if (column == colCount - 1) 
		{
			// 获取相对路径
			QTableWidgetItem* item = table->item(row, column);
			if (!item) return;
			QString relativePath = item->text();

			QLineEdit* dirPathEdit = this->findChild<QLineEdit*>("dirPathEdit");
			if (!dirPathEdit) return;
			QString baseDir = dirPathEdit->text().trimmed();
			if (baseDir.isEmpty()) return;

			//文件的绝对路径
			QDir root(baseDir);
			QString absoluteFilePath = root.absoluteFilePath(relativePath);

			//获取该文件所在的“父目录”路径
			QFileInfo fileInfo(absoluteFilePath);
			QString folderPath = fileInfo.absolutePath(); // 文件所在的文件夹路径

            // 转换为本地路径格式（自动适配 Windows 的 \ 符号和 Linux/Mac 的 / 符号）
			QString localFolder = QDir::toNativeSeparators(folderPath);

			// 拉起 Windows 资源管理器
			QDesktopServices::openUrl(QUrl::fromLocalFile(localFolder));
		}
	});
}

void CodeStatisticsWidget::calculateStatistics()
{
	QTableWidget* tableWidget = this->findChild<QTableWidget*>("tableWidget");
	tableWidget->clearContents();
	tableWidget->setRowCount(0);
	QString path = this->findChild<QLineEdit*>("dirPathEdit")->text().trimmed();
	if (path.isEmpty()) {
		QMessageBox::warning(this, "警告", "请先选择一个有效的文件夹路径!");
		return;
	}

	QDir dirPath(path);
	if (!dirPath.exists())
	{
		QMessageBox::warning(this, "警告", "文件路径不存在!");
		return;
	}

	uint64_t file_count = 0;
	uint64_t bytes_count = 0;
	uint64_t total_lines = 0;
	uint64_t total_code_lines = 0;
	uint64_t total_comment_lines = 0;
	uint64_t total_blank_lines = 0;

	QLineEdit* filterEdit = this->findChild<QLineEdit*>("filterEdit");
	QString filterText = filterEdit->text().trimmed();
	QStringList filters;
	if(!filterText.isEmpty())
	{
		QStringList items = filterText.split(";", Qt::SkipEmptyParts);
		for (QString item : items) {
			item = item.trimmed().toLower(); // 统一转成小写
			// 格式 *.xxx
			if (!item.startsWith("*.")) {
				if (item.startsWith(".")) {
					item = "*" + item;       // .cpp -> *.cpp
				}
				else {
					item = "*." + item;      // cpp -> *.cpp
				}
			}

			if (!filters.contains(item)) {
				filters << item; // 此时filters 内部是 ["*.cpp", "*.h" ...]
			}
		}
	}

	QDirIterator it(path, filters, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext())
	{
		QString filePath = it.next();
		QFileInfo fileInfo(filePath);

		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;// 文件打开失败,跳过
		file_count++;

		bytes_count += fileInfo.size();

		// 单个文件的局部计数器
		//"行数", "代码行数", "注释行数", "空白行数","字符数"
		int file_lines = 0;
		int code_lines = 0;
		int comment_lines = 0;
		int blank_lines = 0;
		int file_chars = 0;
		bool in_block_comment = false; // 用于判定 /* ... */ 多行注释

		QTextStream in(&file);

		while (!in.atEnd())
		{
			QString line = in.readLine();
			file_lines++;
			file_chars += line.length();

			QString trimmedLine = line.trimmed();

			// 优先判定空行
			if (trimmedLine.isEmpty())
			{
				blank_lines++;
				continue;
			}

			// 定义当前行的属性标记
			bool has_code = false;
			bool has_comment = false;

			// 逐字符分析当前行
			int i = 0;
			while (i < trimmedLine.length())
			{
				if (in_block_comment)
				{
					has_comment = true;
					// 检查多行注释是否结束
					if (i + 1 < trimmedLine.length() && trimmedLine[i] == '*' && trimmedLine[i + 1] == '/')
					{
						in_block_comment = false;
						i += 2; // 跳过 */
					}
					else
					{
						i++;
					}
				}
				else
				{
					// 检查单行注释 //
					if (i + 1 < trimmedLine.length() && trimmedLine[i] == '/' && trimmedLine[i + 1] == '/')
					{
						has_comment = true;
						break; // // 后面全是注释，直接结束本行的字符扫描
					}
					// 检查多行注释开始 /*
					else if (i + 1 < trimmedLine.length() && trimmedLine[i] == '/' && trimmedLine[i + 1] == '*')
					{
						has_comment = true;
						in_block_comment = true;
						i += 2; // 跳过 /*
					}
					// 检查是否是有效代码字符（非空白）
					else
					{
						if (!trimmedLine[i].isSpace())
						{
							has_code = true;
						}
						i++;
					}
				}
			}

			// 根据字符扫描结果，精准为本行定性
			if (has_code && has_comment)
			{
				// 既有代码又有注释的混合行
				code_lines++;
				comment_lines++; // 或者根据你的业务标准，混合行只算代码行，这里可以不加
			}
			else if (has_code)
			{
				code_lines++;
			}
			else if (has_comment)
			{
				comment_lines++;
			}
		}
		file.close();

		// 累加总行数
		total_lines += file_lines;
		total_code_lines += code_lines;
		total_comment_lines += comment_lines;
		total_blank_lines += blank_lines;

		// 将当前文件的具体数据动态插入到中间的表格中
		QTableWidget* tableWidget = this->findChild<QTableWidget*>("tableWidget");
		int currentRow = tableWidget->rowCount();
		tableWidget->insertRow(currentRow);

		// "文件名", "类型", "行数", "代码行数", "注释行数", "空白行数","字符数", "文件大小"
		tableWidget->setItem(currentRow, 0, new QTableWidgetItem(fileInfo.fileName()));
		tableWidget->setItem(currentRow, 1, new QTableWidgetItem(fileInfo.suffix()));
		tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::number(file_lines)));
		tableWidget->setItem(currentRow, 3, new QTableWidgetItem(QString::number(code_lines)));
		tableWidget->setItem(currentRow, 4, new QTableWidgetItem(QString::number(comment_lines)));
		tableWidget->setItem(currentRow, 5, new QTableWidgetItem(QString::number(blank_lines)));
		tableWidget->setItem(currentRow, 6, new QTableWidgetItem(QString::number(file_chars)));
		// 文件大小，转换为 KB 并保留两位小数显示
		double kbSize = fileInfo.size() / 1024.0;
		tableWidget->setItem(currentRow, 7, new QTableWidgetItem(QString::asprintf("%.2f KB", kbSize)));

		// 显示相对于选定目录的相对路径
		QString relativePath = QDir(path).relativeFilePath(fileInfo.filePath());
		QTableWidgetItem* pathItem = new QTableWidgetItem(relativePath);
		pathItem->setForeground(QBrush(QColor(30, 144, 255)));
		pathItem->setToolTip("点击打开该文件所在文件夹");
		tableWidget->setItem(currentRow, 8, pathItem);
	}

	this->findChild<QLineEdit*>("fileCountEdit")->setText(QString::number(file_count));
	this->findChild<QLineEdit*>("codeLinesEdit")->setText(QString::number(total_code_lines));
	this->findChild<QLineEdit*>("bytesCountEdit")->setText(QString::number(bytes_count));
	this->findChild<QLineEdit*>("totalLinesEdit")->setText(QString::number(total_lines));
	this->findChild<QLineEdit*>("commentLinesEdit")->setText(QString::number(total_comment_lines));
	this->findChild<QLineEdit*>("blankLinesEdit")->setText(QString::number(total_blank_lines));
}

void CodeStatisticsWidget::onOpenDirectoryClicked()
{
	QString dirPath = QFileDialog::getExistingDirectory(
		this,
		tr("select directory"),
		QString(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
	);
	if (!dirPath.isEmpty())
	{
		this->findChild<QLineEdit*>("dirPathEdit")->setText(dirPath);
	}
}

void CodeStatisticsWidget::onClearClicked()
{
	QTableWidget* tableWidget = this->findChild<QTableWidget*>("tableWidget");
	tableWidget->clearContents();
	tableWidget->setRowCount(0);

	this->findChild<QLineEdit*>("fileCountEdit")->clear();
	this->findChild<QLineEdit*>("codeLinesEdit")->clear();
	this->findChild<QLineEdit*>("dirPathEdit")->clear();
	this->findChild<QLineEdit*>("bytesCountEdit")->clear();
	this->findChild<QLineEdit*>("commentLinesEdit")->clear();
	this->findChild<QLineEdit*>("totalLinesEdit")->clear();
	this->findChild<QLineEdit*>("blankLinesEdit")->clear();

	QLineEdit* filterEdit = this->findChild<QLineEdit*>("filterEdit");
	filterEdit->clear();
	filterEdit->setPlaceholderText("输入文件后缀名,如: *.cpp;*.h");
}

void CodeStatisticsWidget::onAddFilterClicked()
{
	QLineEdit* mainFilterEdit = this->findChild<QLineEdit*>("filterEdit");
	if (!mainFilterEdit) return;

	QDialog* dialog = new QDialog(this);
	dialog->setWindowTitle("编辑过滤后缀");
	dialog->setFixedSize(320, 140);
	dialog->setWindowModality(Qt::WindowModal);

	QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
	QLabel* tipLabel = new QLabel("输入文件后缀 (逗号或空格分隔):", dialog);
	QLineEdit* inputEdit = new QLineEdit(dialog);
	inputEdit->setPlaceholderText("例如: cpp, h, txt");

	//直接从主界面读取文本，去掉通配符符号，还原成干净的输入格式方便用户修改
	QString currentText = mainFilterEdit->text().trimmed(); // 例如 "*.cpp;*.h;"
	if (!currentText.isEmpty()) {
		currentText.replace("*.", "").replace(";", ", ");  // 变成 "cpp, h"
		if (currentText.endsWith(", ")) currentText.chop(2);
		inputEdit->setText(currentText);
	}

	// 按钮布局
	QHBoxLayout* btnLayout = new QHBoxLayout();
	QPushButton* okBtn = new QPushButton("确定", dialog);
	QPushButton* cancelBtn = new QPushButton("取消", dialog);
	btnLayout->addWidget(okBtn);
	btnLayout->addWidget(cancelBtn);

	mainLayout->addWidget(tipLabel);
	mainLayout->addWidget(inputEdit);
	mainLayout->addLayout(btnLayout);

	connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
	connect(okBtn, &QPushButton::clicked, dialog, [=]() {
		QString rawText = inputEdit->text().trimmed().toLower();
		QString resultText = "";

		if (!rawText.isEmpty()) {
			// 支持中文逗号、分号、空格的各种乱入，统一替换成空格切分
			rawText.replace("，", " ").replace(",", " ").replace("；", " ").replace(";", " ");
			QStringList tokens = rawText.split(" ", Qt::SkipEmptyParts);

			QStringList cleanSuffixes;
			for (QString token : tokens) {
				token = token.trimmed();
				if (token.startsWith("*.")) token = token.mid(2);
				if (token.startsWith(".")) token = token.mid(1);

				if (!token.isEmpty() && !cleanSuffixes.contains(token)) {
					cleanSuffixes << token;
				}
			}

			//按要求的标准格式组织： *.cpp;*.h;
			for (const QString& sfx : cleanSuffixes) {
				resultText += "*." + sfx + ";";
			}
		}
		mainFilterEdit->setText(resultText);
		dialog->accept();
	});

	dialog->exec();
}