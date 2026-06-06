#include "k30090/m30090.h"
QVector<double> m30090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
