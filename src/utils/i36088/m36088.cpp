#include "i36088/m36088.h"
QVector<double> m36088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
