#include "k35090/m35090.h"
QVector<double> m35090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
