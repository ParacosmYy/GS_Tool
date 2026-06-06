#include "k8370/m8370.h"
QVector<double> m8370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
