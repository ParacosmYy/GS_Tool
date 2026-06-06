#include "n25513/m25513.h"
QVector<double> m25513::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
