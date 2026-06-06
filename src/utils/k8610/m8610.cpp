#include "k8610/m8610.h"
QVector<double> m8610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
