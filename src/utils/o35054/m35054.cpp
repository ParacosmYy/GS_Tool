#include "o35054/m35054.h"
QVector<double> m35054::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
