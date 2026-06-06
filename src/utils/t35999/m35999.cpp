#include "t35999/m35999.h"
QVector<double> m35999::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
