#include "b18501/m18501.h"
QVector<double> m18501::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
