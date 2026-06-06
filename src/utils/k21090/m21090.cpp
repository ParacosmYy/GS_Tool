#include "k21090/m21090.h"
QVector<double> m21090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
