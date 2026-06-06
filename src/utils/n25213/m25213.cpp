#include "n25213/m25213.h"
QVector<double> m25213::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
