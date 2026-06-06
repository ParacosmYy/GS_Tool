#include "n35073/m35073.h"
QVector<double> m35073::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
