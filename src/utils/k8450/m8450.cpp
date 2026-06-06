#include "k8450/m8450.h"
QVector<double> m8450::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
