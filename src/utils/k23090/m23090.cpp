#include "k23090/m23090.h"
QVector<double> m23090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
