#include "i35688/m35688.h"
QVector<double> m35688::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
