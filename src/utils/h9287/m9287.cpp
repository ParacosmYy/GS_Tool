#include "h9287/m9287.h"
QVector<double> m9287::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
