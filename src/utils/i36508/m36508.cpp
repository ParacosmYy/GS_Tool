#include "i36508/m36508.h"
QVector<double> m36508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
