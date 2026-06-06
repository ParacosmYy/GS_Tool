#include "k18890/m18890.h"
QVector<double> m18890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
