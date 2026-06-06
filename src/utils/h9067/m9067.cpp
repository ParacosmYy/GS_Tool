#include "h9067/m9067.h"
QVector<double> m9067::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
