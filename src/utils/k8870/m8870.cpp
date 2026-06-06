#include "k8870/m8870.h"
QVector<double> m8870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
