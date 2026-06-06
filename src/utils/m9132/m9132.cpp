#include "m9132/m9132.h"
QVector<double> m9132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
