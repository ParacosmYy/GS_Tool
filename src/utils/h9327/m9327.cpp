#include "h9327/m9327.h"
QVector<double> m9327::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
