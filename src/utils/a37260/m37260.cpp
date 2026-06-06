#include "a37260/m37260.h"
QVector<double> m37260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
