#include "i35448/m35448.h"
QVector<double> m35448::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
