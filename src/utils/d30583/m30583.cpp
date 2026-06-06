#include "d30583/m30583.h"
QVector<double> m30583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
