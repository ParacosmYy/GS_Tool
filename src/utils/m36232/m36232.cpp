#include "m36232/m36232.h"
QVector<double> m36232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
