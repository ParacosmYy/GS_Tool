#include "k8310/m8310.h"
QVector<double> m8310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
