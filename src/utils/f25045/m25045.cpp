#include "f25045/m25045.h"
QVector<double> m25045::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
