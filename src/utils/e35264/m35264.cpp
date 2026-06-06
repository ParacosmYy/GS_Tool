#include "e35264/m35264.h"
QVector<double> m35264::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
