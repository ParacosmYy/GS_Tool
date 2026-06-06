#include "m9572/m9572.h"
QVector<double> m9572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
