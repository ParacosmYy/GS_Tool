#include "m35252/m35252.h"
QVector<double> m35252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
