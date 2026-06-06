#include "k25610/m25610.h"
QVector<double> m25610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
