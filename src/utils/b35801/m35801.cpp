#include "b35801/m35801.h"
QVector<double> m35801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
