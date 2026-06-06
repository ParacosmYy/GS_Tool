#include "k8410/m8410.h"
QVector<double> m8410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
