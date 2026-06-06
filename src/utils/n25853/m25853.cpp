#include "n25853/m25853.h"
QVector<double> m25853::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
