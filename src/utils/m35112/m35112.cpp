#include "m35112/m35112.h"
QVector<double> m35112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
