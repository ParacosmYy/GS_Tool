#include "m32352/m32352.h"
QVector<double> m32352::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
