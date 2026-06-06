#include "m18352/m18352.h"
QVector<double> m18352::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
