#include "h9407/m9407.h"
QVector<double> m9407::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
