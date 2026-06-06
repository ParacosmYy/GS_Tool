#include "k8150/m8150.h"
QVector<double> m8150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
