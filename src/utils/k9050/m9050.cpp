#include "k9050/m9050.h"
QVector<double> m9050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
