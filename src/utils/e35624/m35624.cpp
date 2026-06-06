#include "e35624/m35624.h"
QVector<double> m35624::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
