#include "m35292/m35292.h"
QVector<double> m35292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
