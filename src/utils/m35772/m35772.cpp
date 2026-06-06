#include "m35772/m35772.h"
QVector<double> m35772::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
