#include "h9527/m9527.h"
QVector<double> m9527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
