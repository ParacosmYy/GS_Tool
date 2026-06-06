#include "m35352/m35352.h"
QVector<double> m35352::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
