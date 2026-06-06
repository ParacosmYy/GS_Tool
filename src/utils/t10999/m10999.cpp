#include "t10999/m10999.h"
QVector<double> m10999::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
