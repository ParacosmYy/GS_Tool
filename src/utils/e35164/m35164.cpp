#include "e35164/m35164.h"
QVector<double> m35164::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
