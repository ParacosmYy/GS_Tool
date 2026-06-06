#include "m35752/m35752.h"
QVector<double> m35752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
