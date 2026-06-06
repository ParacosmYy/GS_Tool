#include "i8608/m8608.h"
QVector<double> m8608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
