#include "k15890/m15890.h"
QVector<double> m15890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
