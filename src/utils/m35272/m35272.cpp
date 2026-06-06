#include "m35272/m35272.h"
QVector<double> m35272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
